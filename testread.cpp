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

#define FNAME "out.ray"

static void usage(int argc, char **argv)
{
    printf("usage: %s <input file> <output file>\n", argv[0]);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        usage(argc, argv);
        exit(1);
    }

    RayFile file;

    int rc = file.Open(argv[1]);
    if (rc < 0) {
        std::cerr << "error opening file " << argv[1] << std::endl;
        return 1;
    }

    rc = file.ConvertToTGA(argv[2]);
    if (rc < 0) {
        std::cerr << "error outputting file " << argv[2] << std::endl;
        return 1;
    }

    file.Close();

#if 0
    int64_t len = file.GetPixelCount() * 3;
    std::cout << "buffer is " << len * sizeof(float) << " bytes long" << std::endl;
#endif

#if 0
    float *buf = new float[len];

    rc = file.ReadIntoBuffer(buf);
    std::cout << "readintobuffer returns " << rc << std::endl;
#endif

#if 0
    int fd = open((FNAME + std::string(".float")).c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    std::cout << " fd " << fd << std::endl;

    rc = ftruncate(fd, len * sizeof(float));
    if (rc < 0) {
        std::cout << rc << " " << errno << " " << strerror(errno) << std::endl;
        return 1;
    }

    float *buf = (float *)mmap(NULL, len * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    std::cout << "mmapped buf at " << buf << std::endl;

    if (file.ReadIntoBuffer(buf) < 0) {
        std::cout << "error reading in file" << std::endl;
        return -1;
    }

    /* now we have a buffer */

    munmap(buf, len * sizeof(float));

    close(fd);
#endif


    return 0;
}
