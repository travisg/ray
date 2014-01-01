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
#include <iostream>
#include <inttypes.h>
#include "RayFile.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

static void usage(char **argv)
{
    fprintf(stderr, "usage: %s <input file> <x> <y>\n", argv[0]);
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        usage(argv);
        exit(1);
    }

    RayFile file;

    std::string inname(argv[1]);
    printf("infile = %s\n", inname.c_str());

    size_t outpos = inname.find_last_of("/\\");
    std::string outname = inname.substr(outpos + 1);
    printf("outfile = %s\n", outname.c_str());

    int rc = file.Open(argv[1]);
    if (rc < 0) {
        std::cerr << "error opening file " << argv[1] << std::endl;
        return 1;
    }

    int sizex = atoi(argv[2]);
    int sizey = atoi(argv[3]);

    if (sizex <= 0 || sizey <= 0) {
        std::cerr << " invalid X or Y argument" << std::endl;
        return 1;
    }

    float *buf;
    uint64_t len;
    rc = ReadIntoMmap(file, outname + ".float", &buf, &len);
    if (rc < 0) {
        std::cerr << "failed to mmap file" << std::endl;
        return 1;
    }

    printf("sizex %d sizey %d\n", sizex, sizey);
    for (uint32_t y = 0; y < file.Height(); y += sizey) {
        // see how much we can actually do in this chunk
        uint32_t runy = sizey;
        if (y + runy > file.Height())
            runy = file.Height() - y;
        for (uint32_t x = 0; x < file.Width(); x += sizex) {
            RayFile outfile;

            // how much X can we actually do?
            uint32_t runx = sizex;
            if (x + runx > file.Width())
                runx = file.Width() - x;

            char name[512];
            snprintf(name, sizeof(name), "%s-%d.%d", outname.c_str(), x, y);

            printf("outputting %s: size %dx%d, x %d y %d\n", name, runx, runy, x, y);

            outfile.OpenWrite(name, runx, runy);

            for (uint32_t y1 = y; y1 < y + runy; y1++) {
//              printf("setxyrun %d %d %d\n", x, y1, runx);
                outfile.SetXYRun(0, y1 - y, runx, &buf[(y1 * file.Width() + x) * 3]);
            }
            outfile.Close();
        }
    }

    /* close it all down */
    munmap(buf, len);

    return 0;
}
