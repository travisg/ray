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
#include <ray/framebuffer.h>

#include <algorithm>
#include <cstring>

namespace {

void store(float *p, colorf color) {
    p[0] = static_cast<float>(color.r);
    p[1] = static_cast<float>(color.g);
    p[2] = static_cast<float>(color.b);
}

} // namespace

framebuffer::framebuffer(int width, int height)
    : width_(width), height_(height), pixels_(static_cast<size_t>(width) * height * 3) {}

void framebuffer::set_xy(int x, int y, colorf color) {
    std::lock_guard<std::mutex> guard(lock_);
    store(&pixels_[(static_cast<size_t>(y) * width() + x) * 3], color);
    touch(x, y, x + 1, y + 1);
}

void framebuffer::set_xy_run(int x, int y, int count, const colorf *color) {
    std::lock_guard<std::mutex> guard(lock_);
    for (int i = 0; i < count; i++) {
        store(&pixels_[(static_cast<size_t>(y) * width() + x + i) * 3], color[i]);
    }
    touch(x, y, x + count, y + 1);
}

void framebuffer::touch(int x0, int y0, int x1, int y1) {
    if (!dirty_) {
        dx0_ = x0;
        dy0_ = y0;
        dx1_ = x1;
        dy1_ = y1;
        dirty_ = true;
        return;
    }
    dx0_ = std::min(dx0_, x0);
    dy0_ = std::min(dy0_, y0);
    dx1_ = std::max(dx1_, x1);
    dy1_ = std::max(dy1_, y1);
}

std::optional<framebuffer::rect> framebuffer::take_dirty(std::vector<float> &out) {
    std::lock_guard<std::mutex> guard(lock_);
    if (!dirty_) {
        return std::nullopt;
    }
    const rect r{ dx0_, dy0_, dx1_ - dx0_, dy1_ - dy0_ };
    dirty_ = false;
    out.resize(static_cast<size_t>(r.w) * r.h * 3);
    for (int row = 0; row < r.h; row++) {
        memcpy(&out[static_cast<size_t>(row) * r.w * 3],
               &pixels_[(static_cast<size_t>(r.y + row) * width() + r.x) * 3],
               static_cast<size_t>(r.w) * 3 * sizeof(float));
    }
    return r;
}

void framebuffer::mark_all_dirty() {
    std::lock_guard<std::mutex> guard(lock_);
    touch(0, 0, width(), height());
}

colorf framebuffer::at(int x, int y) const {
    const float *p = &pixels_[(static_cast<size_t>(y) * width() + x) * 3];
    return colorf(p[0], p[1], p[2]);
}
