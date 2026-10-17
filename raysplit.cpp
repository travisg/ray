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
#include "ray_file.h"
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool ParseSize(const char *s, uint32_t &out) {
    char *end = nullptr;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v == 0 || v > UINT32_MAX) {
        return false;
    }
    out = static_cast<uint32_t>(v);
    return true;
}

} // namespace

// Splits a .ray file into <basename>-<x>.<y> tiles of at most sizex by sizey
// pixels, written to the current directory.
int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <input file> <x> <y>\n", argv[0]);
        return 1;
    }

    const std::string inname = argv[1];
    const std::string outname = std::filesystem::path(inname).filename().string();
    printf("infile = %s\n", inname.c_str());
    printf("outfile = %s\n", outname.c_str());

    uint32_t sizex, sizey;
    if (!ParseSize(argv[2], sizex) || !ParseSize(argv[3], sizey)) {
        std::cerr << "invalid X or Y argument" << std::endl;
        return 1;
    }

    RayFile file;
    if (file.Open(inname) < 0) {
        std::cerr << "error opening file " << inname << std::endl;
        return 1;
    }

    std::vector<float> buf;
    if (file.ReadIntoBuffer(buf) < 0) {
        std::cerr << "error parsing " << inname << std::endl;
        return 1;
    }

    const uint32_t width = file.Width();
    printf("sizex %u sizey %u\n", sizex, sizey);
    for (uint32_t y = 0; y < file.Height(); y += sizey) {
        const uint32_t runy = std::min(sizey, file.Height() - y);
        for (uint32_t x = 0; x < width; x += sizex) {
            const uint32_t runx = std::min(sizex, width - x);

            const std::string name = outname + "-" + std::to_string(x) + "." + std::to_string(y);
            printf("outputting %s: size %ux%u, x %u y %u\n", name.c_str(), runx, runy, x, y);

            RayFile outfile;
            if (outfile.OpenWrite(name, runx, runy) < 0) {
                std::cerr << "error creating " << name << std::endl;
                return 1;
            }
            for (uint32_t y1 = 0; y1 < runy; y1++) {
                outfile.SetXYRun(0, y1, runx, &buf[(size_t(y + y1) * width + x) * 3]);
            }
            outfile.Close();
        }
    }

    return 0;
}
