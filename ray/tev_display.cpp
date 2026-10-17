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
#include <ray/tev_display.h>

#include <algorithm>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace {

// packet type bytes, from tev's Ipc.h
constexpr uint8_t kCreateImage = 4;
constexpr uint8_t kUpdateImageV2 = 5;

// pixels per side of one update packet
constexpr int kMaxTile = 256;

// A packet: uint32 total length (itself included), the type byte, then little-endian fields;
// strings are NUL terminated.
class packet {
  public:
    explicit packet(uint8_t type, size_t reserve = 64) {
        buf_.reserve(reserve);
        put32(0);
        put8(type);
    }

    void put8(uint8_t v) { buf_.push_back(v); }
    void put32(uint32_t v) {
        for (int i = 0; i < 4; i++) {
            buf_.push_back(static_cast<uint8_t>(v >> (8 * i)));
        }
    }
    void put_float(float f) {
        uint32_t u;
        memcpy(&u, &f, sizeof(u));
        put32(u);
    }
    void put_string(const std::string &s) {
        buf_.insert(buf_.end(), s.begin(), s.end());
        buf_.push_back(0);
    }

    const std::vector<uint8_t> &finish() {
        const auto n = static_cast<uint32_t>(buf_.size());
        for (int i = 0; i < 4; i++) {
            buf_[i] = static_cast<uint8_t>(n >> (8 * i));
        }
        return buf_;
    }

  private:
    std::vector<uint8_t> buf_;
};

void put_channels(packet &p) {
    p.put32(3);
    p.put_string("R");
    p.put_string("G");
    p.put_string("B");
}

} // namespace

tev_display::tev_display(std::string host, std::string port, std::string image_name, int width,
                         int height)
    : host_(std::move(host)), port_(std::move(port)), name_(std::move(image_name)), width_(width),
      height_(height) {}

tev_display::~tev_display() {
    disconnect();
}

void tev_display::disconnect() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool tev_display::connect() {
    if (connected()) {
        return true;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *res = nullptr;
    if (getaddrinfo(host_.c_str(), port_.c_str(), &hints, &res) != 0) {
        return false;
    }
    for (addrinfo *ai = res; ai && fd_ < 0; ai = ai->ai_next) {
        int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            continue;
        }
        timeval timeout{};
        timeout.tv_sec = 3; // a wedged viewer must not stall the render
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#ifdef SO_NOSIGPIPE
        int one = 1;
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
        if (::connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
            fd_ = fd;
        } else {
            close(fd);
        }
    }
    freeaddrinfo(res);
    if (fd_ < 0) {
        return false;
    }

    packet p(kCreateImage);
    p.put8(1); // grab focus: a new image comes to the front
    p.put_string(name_);
    p.put32(static_cast<uint32_t>(width_));
    p.put32(static_cast<uint32_t>(height_));
    put_channels(p);
    return send_all(p.finish());
}

bool tev_display::update(int x, int y, int w, int h, const float *rgb, int pitch) {
    if (!connected()) {
        return false;
    }
    for (int ty = 0; ty < h; ty += kMaxTile) {
        for (int tx = 0; tx < w; tx += kMaxTile) {
            const int tw = std::min(kMaxTile, w - tx);
            const int th = std::min(kMaxTile, h - ty);
            packet p(kUpdateImageV2,
                     64 + name_.size() + static_cast<size_t>(tw) * th * 3 * sizeof(float));
            p.put8(0);
            p.put_string(name_);
            put_channels(p);
            p.put32(static_cast<uint32_t>(x + tx));
            p.put32(static_cast<uint32_t>(y + ty));
            p.put32(static_cast<uint32_t>(tw));
            p.put32(static_cast<uint32_t>(th));
            // one plane per channel
            for (int c = 0; c < 3; c++) {
                for (int row = 0; row < th; row++) {
                    const float *src = rgb + (static_cast<size_t>(ty + row) * pitch + tx) * 3 + c;
                    for (int col = 0; col < tw; col++) {
                        p.put_float(src[static_cast<size_t>(col) * 3]);
                    }
                }
            }
            if (!send_all(p.finish())) {
                return false;
            }
        }
    }
    return true;
}

bool tev_display::send_all(const std::vector<uint8_t> &packet) {
    int flags = 0;
#ifdef MSG_NOSIGNAL
    flags = MSG_NOSIGNAL;
#endif
    size_t sent = 0;
    while (sent < packet.size()) {
        const ssize_t n = send(fd_, packet.data() + sent, packet.size() - sent, flags);
        if (n <= 0) {
            disconnect();
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}
