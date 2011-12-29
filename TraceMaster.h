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
#ifndef __TRACEMASTER_H
#define __TRACEMASTER_H

#include <RenderSurface.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

struct TraceWorkUnit {
	TraceWorkUnit() : startx(0), starty(0), endx(0), endy(0), result(0) {}
	~TraceWorkUnit() { if (result) delete[] result; }

	int startx, starty;
	int endx, endy;

	/* array, rows first */
	colorf *result;

	/* some accessors */
	void SetResult(int x, int y, colorf c) {
		result[(y - starty) * Width() + (x - startx)] = c;
	}
	colorf GetResult(int x, int y) {
		return result[(y - starty) * Width() + (x - startx)];
	}

	int Width() const { return endx + 1 - startx; }
	int Height() const { return endy + 1 - starty; }
};

class TraceMaster {
public:
	TraceMaster(RenderSurface &surface);
	virtual ~TraceMaster();

	virtual int GetWorkUnit(TraceWorkUnit &) = 0;
	virtual int ReturnWorkUnit(TraceWorkUnit &) = 0;

	void WaitForDone();

protected:
	void Lock() { m_Lock.lock(); }
	void Unlock() { m_Lock.unlock(); }
	bool Isdone() const { return m_Done; }
	void SetDone();
	RenderSurface &GetSurface() { return m_Surface; }

private:
	boost::mutex m_Lock;
	boost::condition_variable m_DoneCond;

	RenderSurface &m_Surface;
	bool m_Done;
};

// simple tracemaster that breaks the work into chunks and hands them out in order
class TraceMasterSimple : public TraceMaster {
public:
	TraceMasterSimple(RenderSurface &surface);
	virtual ~TraceMasterSimple();

	virtual int GetWorkUnit(TraceWorkUnit &);
	virtual int ReturnWorkUnit(TraceWorkUnit &);

private:
	int m_Nextx, m_Nexty;
	bool m_IssuedLast;
	int m_PendingCount;
};

class TraceMasterRandom : public TraceMaster {
public:
	TraceMasterRandom(RenderSurface &surface);
	virtual ~TraceMasterRandom();

	virtual int GetWorkUnit(TraceWorkUnit &);
	virtual int ReturnWorkUnit(TraceWorkUnit &);

private:
	bool *m_Bitmap;
	int m_Count;
	int m_PendingCount;
};



#endif

