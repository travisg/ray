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
#include <cstring>
#include <iostream>
#include <ray/ray_file.h>

namespace {

constexpr size_t kReadChunk = 64 * 1024;

template <typename T> T peek(const std::vector<uint8_t> &buf, size_t pos) {
    T v;
    std::memcpy(&v, &buf[pos], sizeof(T));
    return v;
}

} // namespace

bool ray_stream_reader::fill() {
    if (pos_ > 0) {
        buf_.erase(buf_.begin(), buf_.begin() + pos_);
        pos_ = 0;
    }
    // a FILE* that hit EOF earlier sees bytes appended since only after clearerr
    clearerr(fp_);
    size_t old = buf_.size();
    buf_.resize(old + kReadChunk);
    size_t n = fread(&buf_[old], 1, kReadChunk, fp_);
    buf_.resize(old + n);
    return n > 0;
}

bool ray_stream_reader::need(size_t n) {
    while (buf_.size() - pos_ < n) {
        if (!fill()) {
            return false;
        }
    }
    return true;
}

ray_stream_reader::status ray_stream_reader::read_header() {
    if (have_header_) {
        return status::ok;
    }
    if (!need(sizeof(ray_header))) {
        return status::need_data;
    }
    auto header = peek<ray_header>(buf_, pos_);
    if (header.magic != kRayHeaderMagic) {
        return status::error;
    }
    width_ = header.width;
    height_ = header.height;
    pos_ += sizeof(ray_header);
    have_header_ = true;
    return status::ok;
}

ray_stream_reader::status ray_stream_reader::pump(const ray_run_callback &cb) {
    if (!have_header_) {
        return status::error;
    }
    for (;;) {
        if (!need(sizeof(uint32_t))) {
            return status::need_data;
        }
        switch (static_cast<ray_data_type>(peek<uint32_t>(buf_, pos_))) {
            case ray_data_type::pixel: {
                if (!need(sizeof(uint32_t) + sizeof(ray_data_pixel))) {
                    return status::need_data;
                }
                auto pixel = peek<ray_data_pixel>(buf_, pos_ + sizeof(uint32_t));
                if (pixel.x >= width_ || pixel.y >= height_) {
                    return status::error;
                }
                const float rgb[3] = { pixel.r, pixel.g, pixel.b };
                cb(pixel.x, pixel.y, 1, rgb);
                pos_ += sizeof(uint32_t) + sizeof(ray_data_pixel);
                break;
            }
            case ray_data_type::pixel_run: {
                if (!need(sizeof(uint32_t) + sizeof(ray_data_pixel_run))) {
                    return status::need_data;
                }
                auto run = peek<ray_data_pixel_run>(buf_, pos_ + sizeof(uint32_t));
                if (run.x >= width_ || run.y >= height_ || run.length == 0 ||
                    run.length > width_ - run.x) {
                    return status::error;
                }
                size_t bytes = size_t(run.length) * 3 * sizeof(float);
                if (!need(sizeof(uint32_t) + sizeof(ray_data_pixel_run) + bytes)) {
                    return status::need_data;
                }
                run_.resize(size_t(run.length) * 3);
                std::memcpy(run_.data(),
                            &buf_[pos_ + sizeof(uint32_t) + sizeof(ray_data_pixel_run)], bytes);
                cb(run.x, run.y, run.length, run_.data());
                pos_ += sizeof(uint32_t) + sizeof(ray_data_pixel_run) + bytes;
                break;
            }
            case ray_data_type::eof:
                pos_ += sizeof(uint32_t);
                return status::eof;
            case ray_data_type::null:
                pos_ += sizeof(uint32_t);
                break;
            default:
                return status::error;
        }
    }
}

ray_file::~ray_file() {
    close();
}

int ray_file::open(const std::string &name) {
    close();

    fp_ = fopen(name.c_str(), "rb");
    if (!fp_) {
        return -1;
    }

    ray_stream_reader reader(fp_);
    if (reader.read_header() != ray_stream_reader::status::ok) {
        close();
        return -1;
    }
    width_ = reader.width();
    height_ = reader.height();
    return 0;
}

int ray_file::open_write(const std::string &name, uint32_t width, uint32_t height) {
    close();

    fp_ = fopen(name.c_str(), "wb");
    if (!fp_) {
        return -1;
    }

    write_ = true;
    width_ = width;
    height_ = height;

    const ray_header header = { kRayHeaderMagic, width, height };
    fwrite(&header, sizeof(header), 1, fp_);
    return 0;
}

void ray_file::close() {
    if (!fp_) {
        return;
    }
    if (write_) {
        const uint32_t type = static_cast<uint32_t>(ray_data_type::eof);
        fwrite(&type, sizeof(type), 1, fp_);
    }
    fclose(fp_);
    fp_ = nullptr;
    write_ = false;
}

void ray_file::set_xy(int x, int y, colorf color) {
    if (!fp_ || !write_) {
        return;
    }
    const uint32_t type = static_cast<uint32_t>(ray_data_type::pixel);
    const ray_data_pixel pixel = { uint32_t(x), uint32_t(y), float(color.r), float(color.g),
                                   float(color.b) };
    fwrite(&type, sizeof(type), 1, fp_);
    fwrite(&pixel, sizeof(pixel), 1, fp_);
}

void ray_file::write_run(uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
    const uint32_t header[4] = { static_cast<uint32_t>(ray_data_type::pixel_run), x, y, count };
    fwrite(header, sizeof(header), 1, fp_);
    fwrite(rgb, 3 * sizeof(float), count, fp_);
}

void ray_file::set_xy_run(int x, int y, int count, const colorf *color) {
    if (!fp_ || !write_) {
        return;
    }
    scratch_.resize(size_t(count) * 3);
    for (int i = 0; i < count; i++) {
        scratch_[i * 3] = color[i].r;
        scratch_[i * 3 + 1] = color[i].g;
        scratch_[i * 3 + 2] = color[i].b;
    }
    write_run(x, y, count, scratch_.data());
    fflush(fp_);
}

void ray_file::set_xy_run(int x, int y, int count, const float *rgb) {
    if (!fp_ || !write_) {
        return;
    }
    write_run(x, y, count, rgb);
}

int ray_file::parse(const ray_run_callback &cb) {
    if (!fp_ || write_) {
        return -1;
    }
    rewind(fp_);
    ray_stream_reader reader(fp_);
    if (reader.read_header() != ray_stream_reader::status::ok) {
        return -1;
    }
    return reader.pump(cb) == ray_stream_reader::status::error ? -1 : 0;
}

int ray_file::read_into_buffer(std::vector<float> &buf) {
    const size_t width = width_;
    buf.assign(pixel_count() * 3, 0.0f);
    return parse([&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
        std::memcpy(&buf[(y * width + x) * 3], rgb, size_t(count) * 3 * sizeof(float));
    });
}

int convert_to_tga(ray_file &file, const std::string &filename) {
    const uint32_t width = file.width();
    const uint32_t height = file.height();
    if (width > 0xffff || height > 0xffff) {
        std::cerr << "image too large for TGA: " << width << "x" << height << std::endl;
        return -1;
    }

    // BGR, top-left origin
    std::vector<uint8_t> image(size_t(width) * height * 3, 0);
    int rc = file.parse([&](uint32_t x, uint32_t y, uint32_t count, const float *rgb) {
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
