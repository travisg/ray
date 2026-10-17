/*
 * Copyright (c) 2008-2011 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// Displays a .ray file as it is written, polling the file for new packets.

#include <SDL.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <ray/color.h>
#include <ray/ray_file.h>

namespace {

// Renders up to this size open a 1:1 window; larger ones are scaled.
constexpr int kMaxNativeWidth = 2048;
constexpr int kMaxNativeHeight = 1600;
constexpr int kFallbackWidth = 800;
constexpr int kFallbackHeight = 600;

constexpr Uint32 kRefreshMs = 100;
constexpr auto kTailPoll = std::chrono::milliseconds(100);
constexpr auto kHeaderTimeout = std::chrono::seconds(10);

struct sdl_deleter {
    void operator()(SDL_Window *p) const { SDL_DestroyWindow(p); }
    void operator()(SDL_Renderer *p) const { SDL_DestroyRenderer(p); }
    void operator()(SDL_Texture *p) const { SDL_DestroyTexture(p); }
};
template <typename T> using sdl_ptr = std::unique_ptr<T, sdl_deleter>;

struct file_closer {
    void operator()(FILE *fp) const { fclose(fp); }
};

// Pixels received so far plus the rectangle touched since the last upload.
// set_run runs on the reader thread; consume_dirty on the main thread.
class frame {
  public:
    frame(uint32_t width, uint32_t height)
        : width_(width), height_(height), pixels_(size_t(width) * height, 0) {}

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    bool dirty() const { return dirty_.load(); }

    void set_run(uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        if (x >= width_ || y >= height_) {
            return;
        }
        count = std::min(count, width_ - x);

        std::lock_guard<std::mutex> guard(lock_);
        uint32_t *out = &pixels_[size_t(y) * width_ + x];
        for (uint32_t i = 0; i < count; i++) {
            out[i] = color32(colorf(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2])).rgbx();
        }
        const SDL_Rect touched = { int(x), int(y), int(count), 1 };
        if (dirty_) {
            SDL_UnionRect(&rect_, &touched, &rect_);
        } else {
            rect_ = touched;
            dirty_ = true;
        }
    }

    // Calls f(rect, pixels of the rect's first row, pitch) under the lock and
    // clears the rectangle; false when nothing changed since the last call.
    bool consume_dirty(const std::function<void(const SDL_Rect &, const void *, int)> &f) {
        std::lock_guard<std::mutex> guard(lock_);
        if (!dirty_) {
            return false;
        }
        f(rect_, &pixels_[size_t(rect_.y) * width_ + rect_.x], int(width_ * sizeof(uint32_t)));
        dirty_ = false;
        return true;
    }

  private:
    const uint32_t width_;
    const uint32_t height_;
    std::vector<uint32_t> pixels_; // RGBX8888
    std::mutex lock_;
    std::atomic<bool> dirty_{ false };
    SDL_Rect rect_{};
};

// Window, renderer and the streaming texture that mirrors a frame.
class viewer {
  public:
    bool create(uint32_t image_width, uint32_t image_height) {
        int w = kFallbackWidth, h = kFallbackHeight;
        if (image_width <= uint32_t(kMaxNativeWidth) &&
            image_height <= uint32_t(kMaxNativeHeight)) {
            w = int(image_width);
            h = int(image_height);
        }
        window_.reset(SDL_CreateWindow("rayview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       w, h, SDL_WINDOW_RESIZABLE));
        if (!window_) {
            fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
            return false;
        }
        renderer_.reset(SDL_CreateRenderer(window_.get(), -1, 0));
        if (!renderer_) {
            fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
            return false;
        }
        texture_.reset(SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_RGBX8888,
                                         SDL_TEXTUREACCESS_STREAMING, int(image_width),
                                         int(image_height)));
        if (!texture_) {
            fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
            return false;
        }
        return true;
    }

    // uploads whatever changed and redraws the whole window
    void present(frame &f) {
        f.consume_dirty([&](const SDL_Rect &rect, const void *pixels, int pitch) {
            SDL_UpdateTexture(texture_.get(), &rect, pixels, pitch);
        });
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer_.get());
        SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer_.get());
    }

  private:
    sdl_ptr<SDL_Window> window_;
    sdl_ptr<SDL_Renderer> renderer_;
    sdl_ptr<SDL_Texture> texture_;
};

// Runs on SDL's timer thread: asks the main loop to redraw when pixels arrived.
Uint32 timer_tick(Uint32 interval, void *param) {
    auto *f = static_cast<frame *>(param);
    if (f->dirty()) {
        SDL_Event event{};
        event.type = SDL_USEREVENT;
        SDL_PushEvent(&event);
    }
    return interval;
}

// Feeds packets into the frame until the EOF packet, an error, or a stop request.
void reader_thread(std::atomic<bool> &stop, ray_stream_reader &reader, frame &f) {
    const ray_run_callback cb = [&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        f.set_run(x, y, count, rgb);
    };
    while (!stop.load()) {
        switch (reader.pump(cb)) {
            case ray_stream_reader::status::need_data:
                std::this_thread::sleep_for(kTailPoll);
                break;
            case ray_stream_reader::status::eof:
                printf("end of render\n");
                return;
            default:
                fprintf(stderr, "malformed .ray stream\n");
                return;
        }
    }
}

bool window_needs_redraw(const SDL_WindowEvent &e) {
    return e.event == SDL_WINDOWEVENT_EXPOSED || e.event == SDL_WINDOWEVENT_RESIZED ||
           e.event == SDL_WINDOWEVENT_SIZE_CHANGED;
}

} // namespace

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [file.ray]\n", argv[0]);
        return 1;
    }
    const char *path = argc == 2 ? argv[1] : "out.ray";

    // ray may not have created the file, or written the header, yet
    const auto deadline = std::chrono::steady_clock::now() + kHeaderTimeout;
    std::unique_ptr<FILE, file_closer> fp;
    std::unique_ptr<ray_stream_reader> reader;
    for (;;) {
        if (!fp) {
            fp.reset(fopen(path, "rb"));
            if (fp) {
                reader = std::make_unique<ray_stream_reader>(fp.get());
            }
        }
        if (reader) {
            const auto status = reader->read_header();
            if (status == ray_stream_reader::status::ok) {
                break;
            }
            if (status == ray_stream_reader::status::error) {
                fprintf(stderr, "%s: not a .ray file\n", path);
                return 1;
            }
        }
        if (std::chrono::steady_clock::now() > deadline) {
            if (!fp) {
                perror(path);
            } else {
                fprintf(stderr, "%s: no header after %lld s\n", path,
                        (long long)std::chrono::seconds(kHeaderTimeout).count());
            }
            return 1;
        }
        std::this_thread::sleep_for(kTailPoll);
    }
    printf("%s: %ux%u\n", path, reader->width(), reader->height());

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    struct sdl_quit {
        ~sdl_quit() { SDL_Quit(); }
    } quit_guard;

    frame f(reader->width(), reader->height());
    viewer v;
    if (!v.create(f.width(), f.height())) {
        return 1;
    }

    std::atomic<bool> stop{ false };
    std::thread worker(reader_thread, std::ref(stop), std::ref(*reader), std::ref(f));
    const SDL_TimerID timer = SDL_AddTimer(kRefreshMs, timer_tick, &f);

    // the event loop stays on the main thread, which macOS requires
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        if (!SDL_WaitEvent(&event)) {
            fprintf(stderr, "SDL_WaitEvent: %s\n", SDL_GetError());
            break;
        }
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                break;
            case SDL_USEREVENT:
                v.present(f);
                break;
            case SDL_WINDOWEVENT:
                if (window_needs_redraw(event.window)) {
                    v.present(f);
                }
                break;
            default:
                break;
        }
    }

    SDL_RemoveTimer(timer);
    stop = true;
    worker.join();
    return 0;
}
