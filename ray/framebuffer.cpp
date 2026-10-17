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

framebuffer::framebuffer(int width, int height)
    : render_surface(width, height), pixels_(size_t(width) * height * 3) {}

void framebuffer::set_xy(int x, int y, colorf color) {
    float *p = &pixels_[(size_t(y) * width() + x) * 3];
    p[0] = static_cast<float>(color.r);
    p[1] = static_cast<float>(color.g);
    p[2] = static_cast<float>(color.b);
}

void framebuffer::set_xy_run(int x, int y, int count, const colorf *color) {
    for (int i = 0; i < count; i++) {
        set_xy(x + i, y, color[i]);
    }
}

colorf framebuffer::at(int x, int y) const {
    const float *p = &pixels_[(size_t(y) * width() + x) * 3];
    return colorf(p[0], p[1], p[2]);
}
