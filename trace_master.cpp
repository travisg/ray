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
#include "trace_master.h"
#include <algorithm>
#include <numeric>
#include <random>

namespace {

int div_round_up(int x, int d) {
    return (x + d - 1) / d;
}

} // namespace

trace_master::trace_master(render_surface &surface, tile_order order, uint32_t seed)
    : surface_(surface), tiles_x_(div_round_up(surface.width(), kWorkUnitSize)),
      tiles_y_(div_round_up(surface.height(), kWorkUnitSize)),
      order_(static_cast<size_t>(tiles_x_) * tiles_y_) {
    std::iota(order_.begin(), order_.end(), uint32_t{ 0 });
    if (order == tile_order::random) {
        std::mt19937 rng(seed);
        std::shuffle(order_.begin(), order_.end(), rng);
    }
    // with no tiles nothing will ever call return_work_unit
    done_ = order_.empty();
}

void trace_master::fill_unit(uint32_t tile, trace_work_unit &unit) const {
    int tx = tile % tiles_x_;
    int ty = tile / tiles_x_;
    unit.startx = tx * kWorkUnitSize;
    unit.starty = ty * kWorkUnitSize;
    unit.endx = std::min(unit.startx + kWorkUnitSize, surface_.width()) - 1;
    unit.endy = std::min(unit.starty + kWorkUnitSize, surface_.height()) - 1;
}

bool trace_master::get_work_unit(trace_work_unit &unit) {
    uint32_t tile;
    {
        std::lock_guard<std::mutex> guard(lock_);
        if (next_ == order_.size()) {
            return false;
        }
        tile = order_[next_++];
        pending_++;
    }
    fill_unit(tile, unit);
    return true;
}

void trace_master::return_work_unit(const trace_work_unit &unit) {
    std::lock_guard<std::mutex> guard(lock_);

    const colorf *c = unit.result.data();
    int runlen = unit.width();
    for (int y = unit.starty; y <= unit.endy; y++) {
        surface_.set_xy_run(unit.startx, y, runlen, c);
        c += runlen;
    }

    pending_--;
    if (next_ == order_.size() && pending_ == 0) {
        done_ = true;
        done_cond_.notify_all();
    }
}

void trace_master::wait_for_done() {
    std::unique_lock<std::mutex> lock(lock_);
    done_cond_.wait(lock, [this] { return done_; });
}
