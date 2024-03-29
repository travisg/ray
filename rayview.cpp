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

#include <sdl.h>
#include <RayFile.h>
#include <Color.h>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

static boost::mutex lock;
static SDL_Surface *screen;
static SDL_TimerID timer;
static int width = 800;
static int height = 600;
static bool resized = false;
static int fd;

static int inwidth, inheight;

static SDL_Surface *surface;

static void WindowUpdate()
{
    lock.lock();
    if (resized) {
        resized = false;
        SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE | SDL_RESIZABLE);
    }

    SDL_Rect srcrect = { 0, 0, inwidth, inheight };
    SDL_Rect dstrect = { 0, 0, width, height };
    SDL_LockSurface(surface);
    SDL_SoftStretch(surface, &srcrect, screen, &dstrect);
    SDL_UnlockSurface(surface);
    lock.unlock();

    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

static Uint32 TimerTick(Uint32 interval, void *)
{
    WindowUpdate();

    return interval;
}

static int SetupSDL()
{
    atexit(SDL_Quit);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    SDL_EnableUNICODE(1);

    return 0;
}

static int OpenWindow()
{
    screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE | SDL_RESIZABLE);
    SDL_UpdateRect(screen, 0,0,0,0); // Update entire surface

    SDL_WM_SetCaption("Ray","ray");

    timer = SDL_AddTimer(1000, &TimerTick, NULL);

    return 0;
}

static ssize_t ReadData(void *_buf, size_t len)
{
    uint8_t *buf = (uint8_t *)_buf;
    size_t pos;

    for (pos = 0; pos < len; ) {
        size_t toread = len - pos;
        ssize_t r = read(fd, &buf[pos], toread);
        if (r < 0)
            return r;

        if ((size_t)r != toread) {
//          printf("read %d, toread %d\n", read, toread);
            usleep(100000);
        }

        pos += r;
    }

    return pos;
}

static int ReaderThread()
{
    for (;;) {
        uint32_t type;

        if (ReadData(&type, sizeof(type)) < 0)
            goto done;
//      printf("type %d\n", type);

        switch (type) {
            case TYPE_PIXEL: {
                RayDataPixel pixel;

                if (ReadData(&pixel, sizeof(pixel)) < 0)
                    goto done;

//              printf("pixel data: x %d y %d, r %f g %f b %f\n", pixel.x, pixel.y, pixel.r, pixel.g, pixel.b);

                colorf cf(pixel.r, pixel.g, pixel.b);
                color32 c32 = cf;

                SDL_LockSurface(surface);
                ((uint32_t *)surface->pixels)[pixel.y * inwidth + pixel.x] = c32.xrgb();
                SDL_UnlockSurface(surface);
                break;
            }
            case TYPE_PIXEL_RUN: {
                RayDataPixelRun run;

                if (ReadData(&run, sizeof(run)) < 0)
                    goto done;

//              printf("pixel run data: x %d y %d, len %d\n", run.x, run.y, run.length);

                for (size_t i = 0; i < run.length; i++) {
                    float c[3];
                    if (ReadData(&c, sizeof(c)) < 0)
                        goto done;

                    colorf cf(c[0], c[1], c[2]);
                    color32 c32 = cf;

                    SDL_LockSurface(surface);
                    ((uint32_t *)surface->pixels)[run.y * inwidth + run.x + i] = c32.xrgb();
                    SDL_UnlockSurface(surface);
                }
                break;
            }
            case TYPE_EOF:
                goto done;
            case TYPE_NULL:
            default:
                ;
        }

    }
done:
    printf("hit EOF\n");

    // stop the timer
    SDL_RemoveTimer(timer);

    // one last window update
    WindowUpdate();

    return 0;
}

int main(int /* argc */, char ** /* argv */)
{
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
    if (header.magic != RAY_HEADER_MAGIC) {
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

    // create the intermediate surface
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, inwidth, inheight, 32,
                                   0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    // start the reader thread
    boost::thread reader(&ReaderThread);

    OpenWindow();

    // main sdl thread loop
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_VIDEORESIZE:
                lock.lock();
                width = event.resize.w;
                height = event.resize.h;
                resized = true;
                lock.unlock();
                WindowUpdate();
                break;
            case SDL_QUIT:
                std::cout << "SDL_QUIT\n";
                quit = true;
                break;
        }
    }

    close(fd);

    return 0;
}

