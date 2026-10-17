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
#include <mutex>
#include <optional>
#include <ray/color.h>
#include <vector>

// Keeps the rendered image in memory as float RGB, row-major, top row first, and tracks the
// rectangle touched since the last take_dirty so a display thread can follow the render.
// Writes and take_dirty lock; at() and data() are for once the writers are done.
class framebuffer {
  public:
    struct rect {
        int x = 0, y = 0, w = 0, h = 0;
    };

    framebuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    void set_xy(int x, int y, colorf color);
    void set_xy_run(int x, int y, int count, const colorf *color);
    // folds a run from render pass `pass` into the running mean: pass 0 stores it, pass k
    // moves each pixel 1/(k+1) of the way toward the new value
    void accumulate_run(int x, int y, int count, const colorf *color, int pass);

    // copies the pixels of the dirty rectangle into out (w * h * 3 floats, row-major) and
    // clears it; nullopt when nothing was written since the last call
    std::optional<rect> take_dirty(std::vector<float> &out);
    void mark_all_dirty();

    colorf at(int x, int y) const;

    // 3 floats per pixel, width() * height() pixels
    const float *data() const { return pixels_.data(); }
    size_t size() const { return pixels_.size(); }

  private:
    void touch(int x0, int y0, int x1, int y1); // exclusive upper bounds, lock_ held

    int width_;
    int height_;
    std::vector<float> pixels_;
    std::mutex lock_;
    bool dirty_ = false;
    int dx0_ = 0, dy0_ = 0, dx1_ = 0, dy1_ = 0; // exclusive upper bounds
};
