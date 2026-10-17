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
#pragma once

#include <cstdint>
#include <ray/cpp.h>
#include <string>
#include <vector>

// Pushes the image to a running tev viewer (https://github.com/Tom94/tev) over its TCP
// protocol: CreateImage once per connection, then UpdateImageV2 packets carrying float RGB
// rectangles. tev is the server, so a missing viewer is a quiet failure and connect() can be
// retried later.
class tev_display {
  public:
    tev_display(std::string host, std::string port, std::string image_name, int width, int height);
    ~tev_display();
    DISALLOW_COPY_ASSIGN_AND_MOVE(tev_display);

    bool connected() const { return fd_ >= 0; }

    // connects and announces the image; false when nothing is listening
    bool connect();
    void disconnect();

    // sends the rectangle (x, y, w, h) of rgb, row-major with pitch pixels per row; false,
    // and disconnected, when the viewer went away
    bool update(int x, int y, int w, int h, const float *rgb, int pitch);

  private:
    bool send_all(const std::vector<uint8_t> &packet);

    std::string host_;
    std::string port_;
    std::string name_;
    int width_;
    int height_;
    int fd_ = -1;
};
