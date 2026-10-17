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

#include <memory>
#include <ray/color.h>
#include <string>

class render_surface {
  public:
    render_surface(int width, int height);
    virtual ~render_surface() = default;

    int width() const { return width_; }
    int height() const { return height_; }

    virtual void set_xy(int x, int y, colorf color) = 0;
    virtual void set_xy_run(int x, int y, int count, const colorf *color) = 0;

  private:
    int width_;
    int height_;
};

class ray_file;

// Streams pixels straight into a .ray file.
class render_surface_file : public render_surface {
  public:
    render_surface_file(int width, int height);
    ~render_surface_file() override;

    int open_out_file(const std::string &name);

    void set_xy(int x, int y, colorf color) override;
    void set_xy_run(int x, int y, int count, const colorf *color) override;

  private:
    std::unique_ptr<ray_file> file_;
};
