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
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

// 'RAY1' stored little-endian: the file starts with the bytes 31 59 41 52.
constexpr uint32_t kRayHeaderMagic = 0x52415931;

struct RayHeader {
    uint32_t magic;
    uint32_t width;
    uint32_t height;
};

enum RayDataType : uint32_t {
    TYPE_NULL = 0,
    TYPE_PIXEL,
    TYPE_PIXEL_RUN,
    TYPE_EOF,
};

struct RayDataPixel {
    uint32_t x;
    uint32_t y;
    float r, g, b;
};

// Followed by length float triples.
struct RayDataPixelRun {
    uint32_t x;
    uint32_t y;
    uint32_t length;
};

static_assert(sizeof(RayHeader) == 12 && sizeof(RayDataPixel) == 20 &&
              sizeof(RayDataPixelRun) == 12);

// Receives one horizontal run of count pixels starting at (x, y); rgb holds
// count * 3 floats.
using RayRunCallback =
    std::function<void(uint32_t x, uint32_t y, uint32_t count, const float *rgb)>;

// Incremental parser over a FILE* that may still be growing: each Pump()
// consumes every complete packet available and stops at the first incomplete
// one, so a viewer can follow a render in progress.
class RayStreamReader {
  public:
    enum class Status { Ok, NeedData, Eof, Error };

    explicit RayStreamReader(FILE *fp) : m_fp(fp) {}

    // Ok once the header is in; NeedData until then; Error on a bad magic.
    Status ReadHeader();
    // NeedData at the end of the data so far, Eof after the EOF packet,
    // Error on a malformed packet.
    Status Pump(const RayRunCallback &cb);

    bool HaveHeader() const { return m_haveHeader; }
    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }

  private:
    // buffers at least n unread bytes, reading more if needed; false if they
    // are not available yet
    bool Need(size_t n);
    bool Fill();

    FILE *m_fp;
    std::vector<uint8_t> m_buf;
    size_t m_pos = 0;
    std::vector<float> m_run;
    bool m_haveHeader = false;
    uint32_t m_width = 0, m_height = 0;
};

class RayFile {
  public:
    RayFile() = default;
    ~RayFile();
    RayFile(const RayFile &) = delete;
    RayFile &operator=(const RayFile &) = delete;

    // reads and checks the header
    int Open(const std::string &name);
    int OpenWrite(const std::string &name, uint32_t width, uint32_t height);
    // writes the EOF packet when the file was opened for writing
    void Close();

    uint64_t GetPixelCount() const { return uint64_t(m_width) * m_height; }
    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }

    void SetXY(int x, int y, colorf color);
    // one write per run, then fflush so a viewer sees it promptly
    void SetXYRun(int x, int y, int count, const colorf *color);
    // one write per run, no flush
    void SetXYRun(int x, int y, int count, const float *rgb);

    // Parses the whole file from the start: 0 at the EOF packet or the end of
    // the data, -1 on a malformed packet.
    int Parse(const RayRunCallback &cb);
    // Resizes buf to width * height * 3 and fills it row-major RGB; pixels
    // the file never wrote stay 0.
    int ReadIntoBuffer(std::vector<float> &buf);

  private:
    void WriteRun(uint32_t x, uint32_t y, uint32_t count, const float *rgb);

    FILE *m_fp = nullptr;
    bool m_write = false;
    uint32_t m_width = 0, m_height = 0;
    std::vector<float> m_scratch;
};

// Writes the image as a 24-bit uncompressed TGA.
int ConvertToTGA(RayFile &file, const std::string &filename);
