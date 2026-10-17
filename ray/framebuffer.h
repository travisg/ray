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

#include <cstddef>
#include <ray/color.h>
#include <ray/render_surface.h>
#include <vector>

// Keeps the rendered image in memory as float RGB, row-major, top row first.
// Writes come through trace_master under its lock, so there is no locking here.
class framebuffer : public render_surface {
  public:
    framebuffer(int width, int height);

    void set_xy(int x, int y, colorf color) override;
    void set_xy_run(int x, int y, int count, const colorf *color) override;

    colorf at(int x, int y) const;

    // 3 floats per pixel, width() * height() pixels
    const float *data() const { return pixels_.data(); }
    size_t size() const { return pixels_.size(); }

  private:
    std::vector<float> pixels_;
};
