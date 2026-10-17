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

#include <array>
#include <condition_variable>
#include <cpp.h>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <render_surface.h>
#include <vector>

// Tiles are kWorkUnitSize x kWorkUnitSize pixels, clipped at the image edge.
inline constexpr int kWorkUnitSize = 16;

struct trace_work_unit {
    // inclusive bounds
    int startx = 0, starty = 0;
    int endx = 0, endy = 0;

    // row-major, width() per row
    std::array<colorf, kWorkUnitSize * kWorkUnitSize> result;

    void set_result(int x, int y, colorf c) { result[(y - starty) * width() + (x - startx)] = c; }

    int width() const { return endx + 1 - startx; }
    int height() const { return endy + 1 - starty; }
};

enum class tile_order { sequential, random };

// Hands out tiles to the tracer threads and writes finished tiles to the surface.
class trace_master {
  public:
    trace_master(render_surface &surface, tile_order order, uint32_t seed);
    DISALLOW_COPY_ASSIGN_AND_MOVE(trace_master);

    // false once every tile has been issued
    bool get_work_unit(trace_work_unit &unit);
    // the last tile returned wakes wait_for_done
    void return_work_unit(const trace_work_unit &unit);
    void wait_for_done();

  private:
    void fill_unit(uint32_t tile, trace_work_unit &unit) const;

    render_surface &surface_;
    const int tiles_x_;
    const int tiles_y_;
    std::vector<uint32_t> order_; // tile indices in issue order

    // guards everything below and the surface writes (ray_file is not thread-safe)
    std::mutex lock_;
    std::condition_variable done_cond_;
    size_t next_ = 0;
    int pending_ = 0;
    bool done_ = false;
};
