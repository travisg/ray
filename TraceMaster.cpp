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
#include <iostream>
#include <mutex>
#include "TraceMaster.h"

#define WORKUNITSIZE 16

TraceMaster::TraceMaster(RenderSurface &surface)
    :   m_Surface(surface),
        m_Done(false)
{
}

TraceMaster::~TraceMaster()
{
}

void TraceMaster::SetDone()
{
    m_Done = true;
    m_DoneCond.notify_all();
}

void TraceMaster::WaitForDone()
{
    std::unique_lock<std::mutex> lock(m_Lock);
    while (!m_Done) {
        m_DoneCond.wait(lock);
    }
}

// simple tracer
TraceMasterSimple::TraceMasterSimple(RenderSurface &surface)
    :   TraceMaster(surface),
        m_Nextx(0),
        m_Nexty(0),
        m_IssuedLast(false),
        m_PendingCount(0)
{
}

TraceMasterSimple::~TraceMasterSimple()
{
}

int TraceMasterSimple::GetWorkUnit(TraceWorkUnit &unit)
{
    if (m_IssuedLast)
        return -1;

    RenderSurface &surface = GetSurface();

    int oldwidth = unit.Width();
    int oldheight = unit.Height();

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        /* dish out another unit, row first */
        unit.startx = m_Nextx;
        unit.starty = m_Nexty;
        unit.endx = unit.startx + WORKUNITSIZE - 1;
        unit.endy = unit.starty + WORKUNITSIZE - 1;

        m_PendingCount++;

        /* move to the next block */
        m_Nextx += WORKUNITSIZE;
        if (m_Nextx >= surface.Width()) {
            m_Nextx = 0;
            m_Nexty += WORKUNITSIZE;
        }

        if (m_Nexty >= surface.Height())
            m_IssuedLast = true;
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
        unit.result = 0;
    }

    if (!unit.result)
        unit.result = new colorf[WORKUNITSIZE * WORKUNITSIZE];

    return 0;
}

int TraceMasterSimple::ReturnWorkUnit(TraceWorkUnit &unit)
{
    colorf *c = unit.result;

    {
        std::lock_guard<std::mutex> guard(m_Lock);

//      std::cout << "return " << unit.startx << " " << unit.starty << " " << unit.endx << " " << unit.endy << std::endl;
        for (int y = unit.starty; y <= unit.endy; y++) {
            int runlen = unit.endx + 1 - unit.startx;
            GetSurface().SetXYRun(unit.startx, y, runlen, c);
            c += runlen;
        }

        m_PendingCount--;
        if (m_IssuedLast && m_PendingCount == 0)
            SetDone();
    }

    return 0;
}

// random Tracer
TraceMasterRandom::TraceMasterRandom(RenderSurface &surface)
    :   TraceMaster(surface),
        m_PendingCount(0)
{
    m_Count = surface.Width() * surface.Height() / (WORKUNITSIZE * WORKUNITSIZE);
    m_Bitmap = new bool[m_Count];

    for (int i = 0; i < m_Count; i++)
        m_Bitmap[i] = false;
}

TraceMasterRandom::~TraceMasterRandom()
{
    delete[] m_Bitmap;
}

int TraceMasterRandom::GetWorkUnit(TraceWorkUnit &unit)
{
    RenderSurface &surface = GetSurface();

    if (m_Count == 0)
        return -1;

    {
        std::lock_guard<std::mutex> guard(m_Lock);

        for (;;) {
            unsigned int x = rand() % (surface.Width() / WORKUNITSIZE);
            unsigned int y = rand() % (surface.Height() / WORKUNITSIZE);

            if (m_Bitmap[y * (surface.Width() / WORKUNITSIZE) + x] == false) {
                m_Bitmap[y * (surface.Width() / WORKUNITSIZE) + x] = true;
                m_Count--;

                unit.startx = x * WORKUNITSIZE;
                unit.starty = y * WORKUNITSIZE;
                unit.endx = unit.startx + WORKUNITSIZE - 1;
                unit.endy = unit.starty + WORKUNITSIZE - 1;

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
    if (unit.result)
        delete[] unit.result;
    unit.result = new colorf[WORKUNITSIZE * WORKUNITSIZE];

    return 0;
}


int TraceMasterRandom::ReturnWorkUnit(TraceWorkUnit &unit)
{
    colorf *c = unit.result;

    {
        std::lock_guard<std::mutex> guard(m_Lock);

    //  std::cout << "return " << unit.startx << " " << unit.starty << " " << unit.endx << " " << unit.endy << std::endl;
        for (int x = unit.startx; x <= unit.endx; x++) {
            for (int y = unit.starty; y <= unit.endy; y++) {
                GetSurface().SetXY(x, y, *c);
                c++;
            }
        }

        m_PendingCount--;
        if (m_Count == 0 && m_PendingCount == 0)
            SetDone();
    }

    return 0;
}


