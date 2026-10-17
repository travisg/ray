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

#include <RenderSurface.h>
#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

// Tiles are kWorkUnitSize x kWorkUnitSize pixels, clipped at the image edge.
inline constexpr int kWorkUnitSize = 16;

struct TraceWorkUnit {
    // inclusive bounds
    int startx = 0, starty = 0;
    int endx = 0, endy = 0;

    // row-major, Width() per row
    std::array<colorf, kWorkUnitSize * kWorkUnitSize> result;

    void SetResult(int x, int y, colorf c) { result[(y - starty) * Width() + (x - startx)] = c; }

    int Width() const { return endx + 1 - startx; }
    int Height() const { return endy + 1 - starty; }
};

enum class TileOrder { Sequential, Random };

// Hands out tiles to the tracer threads and writes finished tiles to the surface.
class TraceMaster {
  public:
    TraceMaster(RenderSurface &surface, TileOrder order, uint32_t seed);
    TraceMaster(const TraceMaster &) = delete;
    TraceMaster &operator=(const TraceMaster &) = delete;

    // false once every tile has been issued
    bool GetWorkUnit(TraceWorkUnit &unit);
    // the last tile returned wakes WaitForDone
    void ReturnWorkUnit(const TraceWorkUnit &unit);
    void WaitForDone();

  private:
    void FillUnit(uint32_t tile, TraceWorkUnit &unit) const;

    RenderSurface &m_Surface;
    const int m_TilesX;
    const int m_TilesY;
    std::vector<uint32_t> m_Order; // tile indices in issue order

    // guards everything below and the surface writes (RayFile is not thread-safe)
    std::mutex m_Lock;
    std::condition_variable m_DoneCond;
    size_t m_Next = 0;
    int m_Pending = 0;
    bool m_Done = false;
};
