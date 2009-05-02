#include <iostream>
#include "TraceMaster.h"
#include "sdl.h"

#define WORKUNITSIZE 16

TraceMaster::TraceMaster(RenderSurface &surface)
:	m_Surface(surface),
	m_Halt(false)
{
	m_Nextx = 0;
	m_Nexty = 0;

	m_Lock = SDL_CreateMutex();
}

TraceMaster::~TraceMaster()
{
	SDL_DestroyMutex(m_Lock);
}

int TraceMaster::GetWorkUnit(TraceWorkUnit &unit)
{
	if (m_Halt)
		return -1;

	if (m_Nexty >= m_Surface.Height())
		return -1;

	SDL_LockMutex(m_Lock);
	
	/* dish out another unit, row first */
	unit.startx = m_Nextx;
	unit.starty = m_Nexty;
	unit.endx = unit.startx + WORKUNITSIZE - 1;
	unit.endy = unit.starty + WORKUNITSIZE - 1;

	/* move to the next block */
	m_Nextx += WORKUNITSIZE;
	if (m_Nextx >= m_Surface.Width()) {
		m_Nextx = 0;
		m_Nexty += WORKUNITSIZE;
	}

	SDL_UnlockMutex(m_Lock);

	/* trim the result */
	if (unit.endx >= m_Surface.Width()) {
		unit.endx = m_Surface.Width() - 1;
	}
	if (unit.endy >= m_Surface.Height()) {
		unit.endy = m_Surface.Height() - 1;
	}

	/* allocate a new result array */
	if (unit.result)
		delete[] unit.result;
	unit.result = new colorf[WORKUNITSIZE * WORKUNITSIZE];

	return 0;
}

int TraceMaster::ReturnWorkUnit(TraceWorkUnit &unit)
{
	colorf *c = unit.result;

	SDL_LockMutex(m_Lock);

//	std::cout << "return " << unit.startx << " " << unit.starty << " " << unit.endx << " " << unit.endy << std::endl;
	for (int x = unit.startx; x <= unit.endx; x++) {
		for (int y = unit.starty; y <= unit.endy; y++) {
			m_Surface.SetXY(x, y, *c);
			c++;
		}
	}

	SDL_UnlockMutex(m_Lock);

	return 0;
}

void TraceMaster::Halt()
{
	m_Halt = true;
}

