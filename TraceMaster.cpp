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
#include "TraceMaster.h"
#include <iostream>
#include <mutex>

TraceMaster::TraceMaster(RenderSurface &surface) : m_Surface(surface) {}

void TraceMaster::SetDone() {
    m_Done = true;
    m_DoneCond.notify_all();
}

void TraceMaster::WaitForDone() {
    std::unique_lock<std::mutex> lock(m_Lock);
    while (!m_Done) {
        m_DoneCond.wait(lock);
    }
}

// simple tracer
TraceMasterSimple::TraceMasterSimple(RenderSurface &surface) : TraceMaster(surface) {}

int TraceMasterSimple::GetWorkUnit(TraceWorkUnit &unit) {
    if (m_IssuedLast) {
        return -1;
    }

    RenderSurface &surface = GetSurface();

    int oldwidth = unit.Width();
    int oldheight = unit.Height();

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        /* dish out another unit, row first */
        unit.startx = m_Nextx;
        unit.starty = m_Nexty;
        unit.endx = unit.startx + kWorkUnitSize - 1;
        unit.endy = unit.starty + kWorkUnitSize - 1;

        m_PendingCount++;

        /* move to the next block */
        m_Nextx += kWorkUnitSize;
        if (m_Nextx >= surface.Width()) {
            m_Nextx = 0;
            m_Nexty += kWorkUnitSize;
        }

        if (m_Nexty >= surface.Height()) {
            m_IssuedLast = true;
        }
    }

    /* trim the result */
    if (unit.endx >= surface.Width()) {
        unit.endx = surface.Width() - 1;
    }
    if (unit.endy >= surface.Height()) {
        unit.endy = surface.Height() - 1;
    }

    /* allocate a new result array */
    if (unit.result && (oldwidth != unit.Width() || oldheight != unit.Height())) {
        delete[] unit.result;
        unit.result = nullptr;
    }

    if (!unit.result) {
        unit.result = new colorf[kWorkUnitSize * kWorkUnitSize];
    }

    return 0;
}

int TraceMasterSimple::ReturnWorkUnit(TraceWorkUnit &unit) {
    colorf *c = unit.result;

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        for (int y = unit.starty; y <= unit.endy; y++) {
            int runlen = unit.endx + 1 - unit.startx;
            GetSurface().SetXYRun(unit.startx, y, runlen, c);
            c += runlen;
        }

        m_PendingCount--;
        if (m_IssuedLast && m_PendingCount == 0) {
            SetDone();
        }
    }

    return 0;
}

inline int roundup(int x, int roundto) {
    return ((x + roundto - 1) / roundto) * roundto;
}

// random Tracer
TraceMasterRandom::TraceMasterRandom(RenderSurface &surface) : TraceMaster(surface) {
    m_Count = (roundup(surface.Width(), kWorkUnitSize) * roundup(surface.Height(), kWorkUnitSize)) /
              (kWorkUnitSize * kWorkUnitSize);
    m_Bitmap = new bool[m_Count];

    for (int i = 0; i < m_Count; i++) {
        m_Bitmap[i] = false;
    }
}

TraceMasterRandom::~TraceMasterRandom() {
    delete[] m_Bitmap;
}

int TraceMasterRandom::GetWorkUnit(TraceWorkUnit &unit) {
    RenderSurface &surface = GetSurface();

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        if (m_Count == 0) {
            return -1;
        }

        for (;;) {
            unsigned int x = (lrand48() % surface.Width()) / kWorkUnitSize;
            unsigned int y = (lrand48() % surface.Height()) / kWorkUnitSize;

            if (m_Bitmap[y * (roundup(surface.Width(), kWorkUnitSize) / kWorkUnitSize) + x] ==
                false) {
                m_Bitmap[y * (roundup(surface.Width(), kWorkUnitSize) / kWorkUnitSize) + x] = true;
                m_Count--;

                unit.startx = x * kWorkUnitSize;
                unit.starty = y * kWorkUnitSize;
                unit.endx = unit.startx + kWorkUnitSize - 1;
                unit.endy = unit.starty + kWorkUnitSize - 1;

                break;
            }
        }

        m_PendingCount++;
    }

    /* trim the result */
    if (unit.endx >= surface.Width()) {
        unit.endx = surface.Width() - 1;
    }
    if (unit.endy >= surface.Height()) {
        unit.endy = surface.Height() - 1;
    }

    /* allocate a new result array */
    if (unit.result) {
        delete[] unit.result;
    }
    unit.result = new colorf[kWorkUnitSize * kWorkUnitSize];

    return 0;
}

int TraceMasterRandom::ReturnWorkUnit(TraceWorkUnit &unit) {
    colorf *c = unit.result;

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        for (int y = unit.starty; y <= unit.endy; y++) {
            int runlen = unit.endx + 1 - unit.startx;
            GetSurface().SetXYRun(unit.startx, y, runlen, c);
            c += runlen;
        }

        m_PendingCount--;
        if (m_Count == 0 && m_PendingCount == 0) {
            SetDone();
        }
    }

    return 0;
}
