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

int DivRoundUp(int x, int d) {
    return (x + d - 1) / d;
}

} // namespace

TraceMaster::TraceMaster(RenderSurface &surface, TileOrder order, uint32_t seed)
    : m_Surface(surface), m_TilesX(DivRoundUp(surface.Width(), kWorkUnitSize)),
      m_TilesY(DivRoundUp(surface.Height(), kWorkUnitSize)),
      m_Order(static_cast<size_t>(m_TilesX) * m_TilesY) {
    std::iota(m_Order.begin(), m_Order.end(), uint32_t{ 0 });
    if (order == TileOrder::Random) {
        std::mt19937 rng(seed);
        std::shuffle(m_Order.begin(), m_Order.end(), rng);
    }
    // with no tiles nothing will ever call ReturnWorkUnit
    m_Done = m_Order.empty();
}

void TraceMaster::FillUnit(uint32_t tile, TraceWorkUnit &unit) const {
    int tx = tile % m_TilesX;
    int ty = tile / m_TilesX;
    unit.startx = tx * kWorkUnitSize;
    unit.starty = ty * kWorkUnitSize;
    unit.endx = std::min(unit.startx + kWorkUnitSize, m_Surface.Width()) - 1;
    unit.endy = std::min(unit.starty + kWorkUnitSize, m_Surface.Height()) - 1;
}

bool TraceMaster::GetWorkUnit(TraceWorkUnit &unit) {
    uint32_t tile;
    {
        std::lock_guard<std::mutex> guard(m_Lock);
        if (m_Next == m_Order.size()) {
            return false;
        }
        tile = m_Order[m_Next++];
        m_Pending++;
    }
    FillUnit(tile, unit);
    return true;
}

void TraceMaster::ReturnWorkUnit(const TraceWorkUnit &unit) {
    std::lock_guard<std::mutex> guard(m_Lock);

    const colorf *c = unit.result.data();
    int runlen = unit.Width();
    for (int y = unit.starty; y <= unit.endy; y++) {
        m_Surface.SetXYRun(unit.startx, y, runlen, c);
        c += runlen;
    }

    m_Pending--;
    if (m_Next == m_Order.size() && m_Pending == 0) {
        m_Done = true;
        m_DoneCond.notify_all();
    }
}

void TraceMaster::WaitForDone() {
    std::unique_lock<std::mutex> lock(m_Lock);
    m_DoneCond.wait(lock, [this] { return m_Done; });
}
