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
#include <render_surface.h>

#include <ray_file.h>

render_surface::render_surface(int width, int height) : width_(width), height_(height) {}

render_surface_file::render_surface_file(int width, int height) : render_surface(width, height) {}

render_surface_file::~render_surface_file() = default;

int render_surface_file::open_out_file(const std::string &name) {
    file_ = std::make_unique<ray_file>();
    return file_->open_write(name, width(), height());
}

void render_surface_file::set_xy(int x, int y, colorf color) {
    if (file_) {
        file_->set_xy(x, y, color);
    }
}

void render_surface_file::set_xy_run(int x, int y, int count, const colorf *color) {
    if (file_) {
        file_->set_xy_run(x, y, count, color);
    }
}
