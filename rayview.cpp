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
// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include <RayFile.h>
#include <Color.h>

#include <thread>
#include <mutex>

static std::mutex lock;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_TimerID timer;
static int width = 800;
static int height = 600;
static bool resized = false;
static int fd;

static int inwidth, inheight;
static uint32_t *pixels;

static SDL_Texture *tex;

static void WindowUpdate() {
    int err;
    lock.lock();
    if (!window || !renderer) {
        // we haven't opened the window yet
        lock.unlock();
        return;
    }
#if 0
    if (resized) {
        resized = false;
        SDL_SetWindowSize(window, width, height);
    }
#endif
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(tex, nullptr, pixels, inwidth * 4);

    err = SDL_RenderCopy(renderer, tex, NULL, NULL);
    assert(err == 0);

    SDL_RenderPresent(renderer);
    lock.unlock();

    //SDL_FreeSurface(destsurface);
}

static Uint32 TimerTick(Uint32 interval, void *) {
    //printf("tick\n");
    SDL_Event event {};
    SDL_UserEvent userevent {};

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);

    return interval;
}

static int SetupSDL() {
    atexit(SDL_Quit);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    //SDL_EnableUNICODE(1);

    return 0;
}

static int OpenWindow() {
    lock.lock();
    window = SDL_CreateWindow("rayview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    timer = SDL_AddTimer(1000, &TimerTick, NULL);

    lock.unlock();

    return 0;
}

static ssize_t ReadData(void *_buf, size_t len) {
    uint8_t *buf = (uint8_t *)_buf;
    size_t pos;

    for (pos = 0; pos < len; ) {
        size_t toread = len - pos;
        ssize_t r = read(fd, &buf[pos], toread);
        if (r < 0)
            return r;

        // TODO: poll instead of sleeping
        if ((size_t)r != toread) {
//          printf("read %d, toread %d\n", read, toread);
            usleep(100000);
        }

        pos += r;
    }

    return pos;
}

static int ReaderThread() {
    size_t count = 0;
    for (;;) {
        uint32_t type;

        if (ReadData(&type, sizeof(type)) < 0)
            goto done;
        //printf("type %d\n", type);
        count++;
        if ((count % 1000) == 0) {
            printf("reader thread: handled %zu packets\n", count);
        }

        switch (type) {
            case TYPE_PIXEL: {
                RayDataPixel pixel;

                if (ReadData(&pixel, sizeof(pixel)) < 0)
                    goto done;

                //printf("pixel data: x %d y %d, r %f g %f b %f\n", pixel.x, pixel.y, pixel.r, pixel.g, pixel.b);

                colorf cf(pixel.r, pixel.g, pixel.b);
                color32 c32 = cf;

                //printf("pixels %p pixel val %#x\n", pixels, c32.rgbx());
                pixels[pixel.y * inwidth + pixel.x] = c32.rgbx();
                // SDL_UnlockTexture(tex);
                break;
            }
            case TYPE_PIXEL_RUN: {
                RayDataPixelRun run;

                if (ReadData(&run, sizeof(run)) < 0)
                    goto done;

                //printf("pixel run data: x %d y %d, len %d\n", run.x, run.y, run.length);

                for (size_t i = 0; i < run.length; i++) {
                    float c[3];
                    if (ReadData(&c, sizeof(c)) < 0)
                        goto done;

                    colorf cf(c[0], c[1], c[2]);
                    color32 c32 = cf;

                    //printf("pixels %p pixel val %#x\n", pixels, c32.rgbx());
                    pixels[run.y * inwidth + run.x + i] = c32.rgbx();
                }
                break;
            }
            case TYPE_EOF:
                goto done;
            case TYPE_NULL:
            default:
                ;
        }

        // send a SDL_USEREVENT to update the window
        SDL_Event event {};
        SDL_UserEvent userevent {};
        userevent.type = SDL_USEREVENT;
        userevent.code = 0;
        userevent.data1 = NULL;
        userevent.data2 = NULL;
        event.type = SDL_USEREVENT;
        event.user = userevent;
        SDL_PushEvent(&event);

        // XXX remove
        //SDL_Delay(50);
    }
done:
    printf("hit EOF\n");

    // stop the timer
    //SDL_RemoveTimer(timer);

    // one last window update
    WindowUpdate();

    return 0;
}

int main(int /* argc */, char ** /* argv */) {
    printf("hello\n");

    srand(time(NULL));

    // open the input file
    fd = open("out.ray", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "error opening file\n");
        return 1;
    }

    // parse the header of the file
    RayHeader header;
    if (read(fd, &header, sizeof(RayHeader)) < (ssize_t)sizeof(RayHeader)) {
        fprintf(stderr, "ray file corrupt\n");
        return 1;
    }
    if (header.magic != kRayHeaderMagic) {
        fprintf(stderr, "ray file corrupt\n");
        return 1;
    }

    inwidth = header.width;
    inheight = header.height;
    printf("input x y: %d %d\n", inwidth, inheight);

    if (inwidth <= 2048 && inheight <= 1600) {
        width = inwidth;
        height = inheight;
    }

    SetupSDL();

    OpenWindow();

    // create the intermediate surface
    //surface = SDL_CreateRGBSurface(SDL_SWSURFACE, inwidth, inheight, 32,
    //                               0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, inwidth, inheight);
    if (!tex) {
        std::cerr << "failed to allocate texture for rendering" << std::endl;
        abort();
    }
    //SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);

    // allocate a buffer to render into
    pixels = (uint32_t *)calloc(1, inwidth * inheight * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBX8888));

    // start the reader thread
    std::thread reader(&ReaderThread);

    // main sdl thread loop
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    std::cout << "resize x " << std::dec << event.window.data1 << " y " << event.window.data2 << std::endl;
                    lock.lock();
                    width = event.window.data1;
                    height = event.window.data2;
                    resized = true;
                    lock.unlock();
                    WindowUpdate();
                } else {
                    //std::cout << "unhandled SDL_WINDOWEVENT event 0x" << std::hex << (int)event.window.event << std::endl;
                }
                break;
            }
            case SDL_USEREVENT:
                std::cout << "SDL_USEREVENT" << std::endl;
                WindowUpdate();
                break;
            case SDL_QUIT:
                std::cout << "SDL_QUIT" << std::endl;
                quit = true;
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                // eat these
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    std::cout << "SDL_KEYDOWN: escape" << std::endl;
                    quit = true;
                }
                break;
            default:
                std::cout << "unhandled event 0x" << std::hex << event.type << std::endl;
                ;
        }
    }

    reader.join();

    close(fd);

    return 0;
}

