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
#include <condition_variable>
#include <mutex>

// Tiles are kWorkUnitSize x kWorkUnitSize pixels, clipped at the image edge.
inline constexpr int kWorkUnitSize = 16;

struct TraceWorkUnit {
    TraceWorkUnit() = default;
    ~TraceWorkUnit() { delete[] result; }
    TraceWorkUnit(const TraceWorkUnit &) = delete;
    TraceWorkUnit &operator=(const TraceWorkUnit &) = delete;

    // inclusive bounds
    int startx = 0, starty = 0;
    int endx = 0, endy = 0;

    // row-major, Width() per row
    colorf *result = nullptr;

    void SetResult(int x, int y, colorf c) { result[(y - starty) * Width() + (x - startx)] = c; }

    int Width() const { return endx + 1 - startx; }
    int Height() const { return endy + 1 - starty; }
};

// Hands out tiles to the tracer threads and writes finished tiles to the surface.
class TraceMaster {
  public:
    TraceMaster(RenderSurface &surface);
    virtual ~TraceMaster() = default;

    virtual int GetWorkUnit(TraceWorkUnit &) = 0;
    virtual int ReturnWorkUnit(TraceWorkUnit &) = 0;

    void WaitForDone();

  protected:
    void SetDone();
    RenderSurface &GetSurface() { return m_Surface; }

    std::mutex m_Lock;

  private:
    std::condition_variable m_DoneCond;

    RenderSurface &m_Surface;
    bool m_Done = false;
};

// row-major order
class TraceMasterSimple : public TraceMaster {
  public:
    TraceMasterSimple(RenderSurface &surface);

    int GetWorkUnit(TraceWorkUnit &) override;
    int ReturnWorkUnit(TraceWorkUnit &) override;

  private:
    int m_Nextx = 0, m_Nexty = 0;
    bool m_IssuedLast = false;
    int m_PendingCount = 0;
};

// random order
class TraceMasterRandom : public TraceMaster {
  public:
    TraceMasterRandom(RenderSurface &surface);
    ~TraceMasterRandom() override;

    int GetWorkUnit(TraceWorkUnit &) override;
    int ReturnWorkUnit(TraceWorkUnit &) override;

  private:
    bool *m_Bitmap;
    int m_Count;
    int m_PendingCount = 0;
};
