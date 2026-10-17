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
#pragma once

#include "color.h"
#include <cpp.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

// 'RAY1' stored little-endian: the file starts with the bytes 31 59 41 52.
constexpr uint32_t kRayHeaderMagic = 0x52415931;

struct ray_header {
    uint32_t magic;
    uint32_t width;
    uint32_t height;
};

enum class ray_data_type : uint32_t {
    null = 0,
    pixel,
    pixel_run,
    eof,
};

struct ray_data_pixel {
    uint32_t x;
    uint32_t y;
    float r, g, b;
};

// Followed by length float triples.
struct ray_data_pixel_run {
    uint32_t x;
    uint32_t y;
    uint32_t length;
};

static_assert(sizeof(ray_header) == 12 && sizeof(ray_data_pixel) == 20 &&
              sizeof(ray_data_pixel_run) == 12);

// Receives one horizontal run of count pixels starting at (x, y); rgb holds
// count * 3 floats.
using ray_run_callback =
    std::function<void(uint32_t x, uint32_t y, uint32_t count, const float *rgb)>;

// Incremental parser over a FILE* that may still be growing: each pump()
// consumes every complete packet available and stops at the first incomplete
// one, so a viewer can follow a render in progress.
class ray_stream_reader {
  public:
    enum class status { ok, need_data, eof, error };

    explicit ray_stream_reader(FILE *fp) : fp_(fp) {}

    // ok once the header is in; need_data until then; error on a bad magic.
    status read_header();
    // need_data at the end of the data so far, eof after the EOF packet,
    // error on a malformed packet.
    status pump(const ray_run_callback &cb);

    bool have_header() const { return have_header_; }
    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

  private:
    // buffers at least n unread bytes, reading more if needed; false if they
    // are not available yet
    bool need(size_t n);
    bool fill();

    FILE *fp_;
    std::vector<uint8_t> buf_;
    size_t pos_ = 0;
    std::vector<float> run_;
    bool have_header_ = false;
    uint32_t width_ = 0, height_ = 0;
};

class ray_file {
  public:
    ray_file() = default;
    ~ray_file();
    DISALLOW_COPY_ASSIGN_AND_MOVE(ray_file);

    // reads and checks the header
    int open(const std::string &name);
    int open_write(const std::string &name, uint32_t width, uint32_t height);
    // writes the EOF packet when the file was opened for writing
    void close();

    uint64_t pixel_count() const { return uint64_t(width_) * height_; }
    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    void set_xy(int x, int y, colorf color);
    // one write per run, then fflush so a viewer sees it promptly
    void set_xy_run(int x, int y, int count, const colorf *color);
    // one write per run, no flush
    void set_xy_run(int x, int y, int count, const float *rgb);

    // Parses the whole file from the start: 0 at the EOF packet or the end of
    // the data, -1 on a malformed packet.
    int parse(const ray_run_callback &cb);
    // Resizes buf to width * height * 3 and fills it row-major RGB; pixels
    // the file never wrote stay 0.
    int read_into_buffer(std::vector<float> &buf);

  private:
    void write_run(uint32_t x, uint32_t y, uint32_t count, const float *rgb);

    FILE *fp_ = nullptr;
    bool write_ = false;
    uint32_t width_ = 0, height_ = 0;
    std::vector<float> scratch_;
};

// Writes the image as a 24-bit uncompressed TGA.
int convert_to_tga(ray_file &file, const std::string &filename);
