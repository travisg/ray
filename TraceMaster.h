#ifndef __TRACEMASTER_H
#define __TRACEMASTER_H

#include <RenderSurface.h>

struct SDL_mutex;

struct TraceWorkUnit {
	int startx, starty;
	int endx, endy;

	/* array, rows first */
	colorf *result;

	/* some accessors */
	void SetResult(int x, int y, colorf c) {
		result[(y - starty) * (endx - startx) + (x - startx)] = c;
	}

	int Width() const { return endx - startx; }
	int Height() const { return endy - starty; }
};

class TraceMaster {
public:
	TraceMaster(RenderSurface &surface);
	virtual ~TraceMaster();

	int GetWorkUnit(TraceWorkUnit &);
	int ReturnWorkUnit(TraceWorkUnit &);

	int m_Nextx, m_Nexty;

	void Halt();

	SDL_mutex *m_Lock;

private:
	RenderSurface &m_Surface;
	bool m_Halt;
};

#endif

