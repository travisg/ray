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
#include <cstring>
#include <iostream>

namespace {

constexpr size_t kReadChunk = 64 * 1024;

template <typename T> T Peek(const std::vector<uint8_t> &buf, size_t pos) {
    T v;
    std::memcpy(&v, &buf[pos], sizeof(T));
    return v;
}

} // namespace

bool RayStreamReader::Fill() {
    if (m_pos > 0) {
        m_buf.erase(m_buf.begin(), m_buf.begin() + m_pos);
        m_pos = 0;
    }
    // a FILE* that hit EOF earlier sees bytes appended since only after clearerr
    clearerr(m_fp);
    size_t old = m_buf.size();
    m_buf.resize(old + kReadChunk);
    size_t n = fread(&m_buf[old], 1, kReadChunk, m_fp);
    m_buf.resize(old + n);
    return n > 0;
}

bool RayStreamReader::Need(size_t n) {
    while (m_buf.size() - m_pos < n) {
        if (!Fill()) {
            return false;
        }
    }
    return true;
}

RayStreamReader::Status RayStreamReader::ReadHeader() {
    if (m_haveHeader) {
        return Status::Ok;
    }
    if (!Need(sizeof(RayHeader))) {
        return Status::NeedData;
    }
    auto header = Peek<RayHeader>(m_buf, m_pos);
    if (header.magic != kRayHeaderMagic) {
        return Status::Error;
    }
    m_width = header.width;
    m_height = header.height;
    m_pos += sizeof(RayHeader);
    m_haveHeader = true;
    return Status::Ok;
}

RayStreamReader::Status RayStreamReader::Pump(const RayRunCallback &cb) {
    if (!m_haveHeader) {
        return Status::Error;
    }
    for (;;) {
        if (!Need(sizeof(uint32_t))) {
            return Status::NeedData;
        }
        switch (Peek<uint32_t>(m_buf, m_pos)) {
            case TYPE_PIXEL: {
                if (!Need(sizeof(uint32_t) + sizeof(RayDataPixel))) {
                    return Status::NeedData;
                }
                auto pixel = Peek<RayDataPixel>(m_buf, m_pos + sizeof(uint32_t));
                if (pixel.x >= m_width || pixel.y >= m_height) {
                    return Status::Error;
                }
                const float rgb[3] = { pixel.r, pixel.g, pixel.b };
                cb(pixel.x, pixel.y, 1, rgb);
                m_pos += sizeof(uint32_t) + sizeof(RayDataPixel);
                break;
            }
            case TYPE_PIXEL_RUN: {
                if (!Need(sizeof(uint32_t) + sizeof(RayDataPixelRun))) {
                    return Status::NeedData;
                }
                auto run = Peek<RayDataPixelRun>(m_buf, m_pos + sizeof(uint32_t));
                if (run.x >= m_width || run.y >= m_height || run.length == 0 ||
                    run.length > m_width - run.x) {
                    return Status::Error;
                }
                size_t bytes = size_t(run.length) * 3 * sizeof(float);
                if (!Need(sizeof(uint32_t) + sizeof(RayDataPixelRun) + bytes)) {
                    return Status::NeedData;
                }
                m_run.resize(size_t(run.length) * 3);
                std::memcpy(m_run.data(),
                            &m_buf[m_pos + sizeof(uint32_t) + sizeof(RayDataPixelRun)], bytes);
                cb(run.x, run.y, run.length, m_run.data());
                m_pos += sizeof(uint32_t) + sizeof(RayDataPixelRun) + bytes;
                break;
            }
            case TYPE_EOF:
                m_pos += sizeof(uint32_t);
                return Status::Eof;
            case TYPE_NULL:
                m_pos += sizeof(uint32_t);
                break;
            default:
                return Status::Error;
        }
    }
}

RayFile::~RayFile() {
    Close();
}

int RayFile::Open(const std::string &name) {
    Close();

    m_fp = fopen(name.c_str(), "rb");
    if (!m_fp) {
        return -1;
    }

    RayStreamReader reader(m_fp);
    if (reader.ReadHeader() != RayStreamReader::Status::Ok) {
        Close();
        return -1;
    }
    m_width = reader.Width();
    m_height = reader.Height();
    return 0;
}

int RayFile::OpenWrite(const std::string &name, uint32_t width, uint32_t height) {
    Close();

    m_fp = fopen(name.c_str(), "wb");
    if (!m_fp) {
        return -1;
    }

    m_write = true;
    m_width = width;
    m_height = height;

    const RayHeader header = { kRayHeaderMagic, width, height };
    fwrite(&header, sizeof(header), 1, m_fp);
    return 0;
}

void RayFile::Close() {
    if (!m_fp) {
        return;
    }
    if (m_write) {
        const uint32_t type = TYPE_EOF;
        fwrite(&type, sizeof(type), 1, m_fp);
    }
    fclose(m_fp);
    m_fp = nullptr;
    m_write = false;
}

void RayFile::SetXY(int x, int y, colorf color) {
    if (!m_fp || !m_write) {
        return;
    }
    const uint32_t type = TYPE_PIXEL;
    const RayDataPixel pixel = { uint32_t(x), uint32_t(y), float(color.r), float(color.g),
                                 float(color.b) };
    fwrite(&type, sizeof(type), 1, m_fp);
    fwrite(&pixel, sizeof(pixel), 1, m_fp);
}

void RayFile::WriteRun(uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
    const uint32_t header[4] = { TYPE_PIXEL_RUN, x, y, count };
    fwrite(header, sizeof(header), 1, m_fp);
    fwrite(rgb, 3 * sizeof(float), count, m_fp);
}

void RayFile::SetXYRun(int x, int y, int count, const colorf *color) {
    if (!m_fp || !m_write) {
        return;
    }
    m_scratch.resize(size_t(count) * 3);
    for (int i = 0; i < count; i++) {
        m_scratch[i * 3] = color[i].r;
        m_scratch[i * 3 + 1] = color[i].g;
        m_scratch[i * 3 + 2] = color[i].b;
    }
    WriteRun(x, y, count, m_scratch.data());
    fflush(m_fp);
}

void RayFile::SetXYRun(int x, int y, int count, const float *rgb) {
    if (!m_fp || !m_write) {
        return;
    }
    WriteRun(x, y, count, rgb);
}

int RayFile::Parse(const RayRunCallback &cb) {
    if (!m_fp || m_write) {
        return -1;
    }
    rewind(m_fp);
    RayStreamReader reader(m_fp);
    if (reader.ReadHeader() != RayStreamReader::Status::Ok) {
        return -1;
    }
    return reader.Pump(cb) == RayStreamReader::Status::Error ? -1 : 0;
}

int RayFile::ReadIntoBuffer(std::vector<float> &buf) {
    const size_t width = m_width;
    buf.assign(GetPixelCount() * 3, 0.0f);
    return Parse([&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        std::memcpy(&buf[(y * width + x) * 3], rgb, size_t(count) * 3 * sizeof(float));
    });
}

int ConvertToTGA(RayFile &file, const std::string &filename) {
    const uint32_t width = file.Width();
    const uint32_t height = file.Height();
    if (width > 0xffff || height > 0xffff) {
        std::cerr << "image too large for TGA: " << width << "x" << height << std::endl;
        return -1;
    }

    // BGR, top-left origin
    std::vector<uint8_t> image(size_t(width) * height * 3, 0);
    int rc = file.Parse([&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        uint8_t *out = &image[(size_t(y) * width + x) * 3];
        for (uint32_t i = 0; i < count; i++) {
            const color32 c(colorf(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]));
            out[i * 3] = c.b;
            out[i * 3 + 1] = c.g;
            out[i * 3 + 2] = c.r;
        }
    });
    if (rc < 0) {
        std::cerr << "error parsing ray file" << std::endl;
        return -1;
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cerr << "error opening tga file " << filename << std::endl;
        return -1;
    }

    const uint8_t header[18] = {
        0, // id length
        0, // no colour map
        2, // uncompressed true colour
        0,
        0,
        0,
        0,
        0, // colour map spec
        0,
        0,
        0,
        0, // x, y origin
        uint8_t(width & 0xff),
        uint8_t(width >> 8), // width
        uint8_t(height & 0xff),
        uint8_t(height >> 8), // height
        24,                   // bits per pixel
        0x20,                 // top-left origin
    };
    fwrite(header, sizeof(header), 1, fp);
    fwrite(image.data(), 1, image.size(), fp);
    fclose(fp);
    return 0;
}
