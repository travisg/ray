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

#include <Color.h>
#include <RayFile.h>

namespace {

// Renders up to this size open a 1:1 window; larger ones are scaled.
constexpr int kMaxNativeWidth = 2048;
constexpr int kMaxNativeHeight = 1600;
constexpr int kFallbackWidth = 800;
constexpr int kFallbackHeight = 600;

constexpr Uint32 kRefreshMs = 100;
constexpr auto kTailPoll = std::chrono::milliseconds(100);
constexpr auto kHeaderTimeout = std::chrono::seconds(10);

struct SdlDeleter {
    void operator()(SDL_Window *p) const { SDL_DestroyWindow(p); }
    void operator()(SDL_Renderer *p) const { SDL_DestroyRenderer(p); }
    void operator()(SDL_Texture *p) const { SDL_DestroyTexture(p); }
};
template <typename T> using SdlPtr = std::unique_ptr<T, SdlDeleter>;

struct FileCloser {
    void operator()(FILE *fp) const { fclose(fp); }
};

// Pixels received so far plus the rectangle touched since the last upload.
// SetRun runs on the reader thread; ConsumeDirty on the main thread.
class Frame {
  public:
    Frame(uint32_t width, uint32_t height)
        : m_width(width), m_height(height), m_pixels(size_t(width) * height, 0) {}

    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }
    bool Dirty() const { return m_dirty.load(); }

    void SetRun(uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        if (x >= m_width || y >= m_height) {
            return;
        }
        count = std::min(count, m_width - x);

        std::lock_guard<std::mutex> guard(m_lock);
        uint32_t *out = &m_pixels[size_t(y) * m_width + x];
        for (uint32_t i = 0; i < count; i++) {
            out[i] = color32(colorf(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2])).rgbx();
        }
        const SDL_Rect touched = { int(x), int(y), int(count), 1 };
        if (m_dirty) {
            SDL_UnionRect(&m_rect, &touched, &m_rect);
        } else {
            m_rect = touched;
            m_dirty = true;
        }
    }

    // Calls f(rect, pixels of the rect's first row, pitch) under the lock and
    // clears the rectangle; false when nothing changed since the last call.
    bool ConsumeDirty(const std::function<void(const SDL_Rect &, const void *, int)> &f) {
        std::lock_guard<std::mutex> guard(m_lock);
        if (!m_dirty) {
            return false;
        }
        f(m_rect, &m_pixels[size_t(m_rect.y) * m_width + m_rect.x],
          int(m_width * sizeof(uint32_t)));
        m_dirty = false;
        return true;
    }

  private:
    const uint32_t m_width;
    const uint32_t m_height;
    std::vector<uint32_t> m_pixels; // RGBX8888
    std::mutex m_lock;
    std::atomic<bool> m_dirty{ false };
    SDL_Rect m_rect{};
};

// Window, renderer and the streaming texture that mirrors a Frame.
class Viewer {
  public:
    bool Create(uint32_t imageWidth, uint32_t imageHeight) {
        int w = kFallbackWidth, h = kFallbackHeight;
        if (imageWidth <= uint32_t(kMaxNativeWidth) && imageHeight <= uint32_t(kMaxNativeHeight)) {
            w = int(imageWidth);
            h = int(imageHeight);
        }
        m_window.reset(SDL_CreateWindow("rayview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        w, h, SDL_WINDOW_RESIZABLE));
        if (!m_window) {
            fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
            return false;
        }
        m_renderer.reset(SDL_CreateRenderer(m_window.get(), -1, 0));
        if (!m_renderer) {
            fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
            return false;
        }
        m_texture.reset(SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBX8888,
                                          SDL_TEXTUREACCESS_STREAMING, int(imageWidth),
                                          int(imageHeight)));
        if (!m_texture) {
            fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
            return false;
        }
        return true;
    }

    // uploads whatever changed and redraws the whole window
    void Present(Frame &frame) {
        frame.ConsumeDirty([&](const SDL_Rect &rect, const void *pixels, int pitch) {
            SDL_UpdateTexture(m_texture.get(), &rect, pixels, pitch);
        });
        SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(m_renderer.get());
        SDL_RenderCopy(m_renderer.get(), m_texture.get(), nullptr, nullptr);
        SDL_RenderPresent(m_renderer.get());
    }

  private:
    SdlPtr<SDL_Window> m_window;
    SdlPtr<SDL_Renderer> m_renderer;
    SdlPtr<SDL_Texture> m_texture;
};

// Runs on SDL's timer thread: asks the main loop to redraw when pixels arrived.
Uint32 TimerTick(Uint32 interval, void *param) {
    auto *frame = static_cast<Frame *>(param);
    if (frame->Dirty()) {
        SDL_Event event{};
        event.type = SDL_USEREVENT;
        SDL_PushEvent(&event);
    }
    return interval;
}

// Feeds packets into the frame until the EOF packet, an error, or a stop request.
void ReaderThread(std::atomic<bool> &stop, RayStreamReader &reader, Frame &frame) {
    const RayRunCallback cb = [&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        frame.SetRun(x, y, count, rgb);
    };
    while (!stop.load()) {
        switch (reader.Pump(cb)) {
            case RayStreamReader::Status::NeedData:
                std::this_thread::sleep_for(kTailPoll);
                break;
            case RayStreamReader::Status::Eof:
                printf("end of render\n");
                return;
            default:
                fprintf(stderr, "malformed .ray stream\n");
                return;
        }
    }
}

bool WindowNeedsRedraw(const SDL_WindowEvent &e) {
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
    std::unique_ptr<FILE, FileCloser> fp;
    std::unique_ptr<RayStreamReader> reader;
    for (;;) {
        if (!fp) {
            fp.reset(fopen(path, "rb"));
            if (fp) {
                reader = std::make_unique<RayStreamReader>(fp.get());
            }
        }
        if (reader) {
            const auto status = reader->ReadHeader();
            if (status == RayStreamReader::Status::Ok) {
                break;
            }
            if (status == RayStreamReader::Status::Error) {
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
    printf("%s: %ux%u\n", path, reader->Width(), reader->Height());

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    struct SdlQuit {
        ~SdlQuit() { SDL_Quit(); }
    } sdlQuit;

    Frame frame(reader->Width(), reader->Height());
    Viewer viewer;
    if (!viewer.Create(frame.Width(), frame.Height())) {
        return 1;
    }

    std::atomic<bool> stop{ false };
    std::thread readerThread(ReaderThread, std::ref(stop), std::ref(*reader), std::ref(frame));
    const SDL_TimerID timer = SDL_AddTimer(kRefreshMs, TimerTick, &frame);

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
                viewer.Present(frame);
                break;
            case SDL_WINDOWEVENT:
                if (WindowNeedsRedraw(event.window)) {
                    viewer.Present(frame);
                }
                break;
            default:
                break;
        }
    }

    SDL_RemoveTimer(timer);
    stop = true;
    readerThread.join();
    return 0;
}
