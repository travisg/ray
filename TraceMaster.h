#ifndef __TRACEMASTER_H
#define __TRACEMASTER_H

#include <RenderSurface.h>
#include "sdl.h"

struct SDL_mutex;

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

	void Halt();
	void WaitForDone();

protected:
	void Lock() { SDL_LockMutex(m_Lock); }
	void Unlock() { SDL_UnlockMutex(m_Lock); }
	bool IsHalted() const { return m_Halt; }
	bool Isdone() const { return m_Done; }
	void SetDone() { m_Done = true; }
	RenderSurface &GetSurface() { return m_Surface; }

private:
	SDL_mutex *m_Lock;

	RenderSurface &m_Surface;
	bool m_Halt;
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

