/*
 * Copyright (c) 2026 Travis Geiselbrecht
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
#include <ray/pfm.h>

#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

namespace {

constexpr bool kLittleEndian = std::endian::native == std::endian::little;

struct file_closer {
    void operator()(FILE *fp) const { fclose(fp); }
};
using file_ptr = std::unique_ptr<FILE, file_closer>;

void swap_floats(float *v, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uint32_t u;
        memcpy(&u, &v[i], 4);
        u = __builtin_bswap32(u);
        memcpy(&v[i], &u, 4);
    }
}

} // namespace

bool write_pfm(const std::string &path, int width, int height, const float *rgb) {
    file_ptr fp(fopen(path.c_str(), "wb"));
    if (!fp) {
        return false;
    }
    if (fprintf(fp.get(), "PF\n%d %d\n%s\n", width, height, kLittleEndian ? "-1.0" : "1.0") < 0) {
        return false;
    }
    const size_t row = static_cast<size_t>(width) * 3;
    for (int y = height - 1; y >= 0; y--) {
        if (fwrite(rgb + static_cast<size_t>(y) * row, sizeof(float), row, fp.get()) != row) {
            return false;
        }
    }
    return fflush(fp.get()) == 0;
}

bool read_pfm(const std::string &path, int &width, int &height, std::vector<float> &rgb) {
    file_ptr fp(fopen(path.c_str(), "rb"));
    if (!fp) {
        return false;
    }
    char magic[3] = {};
    double scale = 0;
    // the header ends with exactly one whitespace character before the data
    if (fscanf(fp.get(), "%2s %d %d %lf", magic, &width, &height, &scale) != 4 ||
        strcmp(magic, "PF") != 0 || width <= 0 || height <= 0 || scale == 0 ||
        fgetc(fp.get()) == EOF) {
        return false;
    }
    const size_t row = static_cast<size_t>(width) * 3;
    rgb.assign(row * height, 0.0f);
    for (int y = height - 1; y >= 0; y--) {
        if (fread(rgb.data() + static_cast<size_t>(y) * row, sizeof(float), row, fp.get()) != row) {
            return false;
        }
    }
    if ((scale < 0) != kLittleEndian) {
        swap_floats(rgb.data(), rgb.size());
    }
    return true;
}
