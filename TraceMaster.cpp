#include <iostream>
#include "TraceMaster.h"
#include "sdl.h"

#define WORKUNITSIZE 128

TraceMaster::TraceMaster(RenderSurface &surface)
:	m_Surface(surface),
	m_Halt(false),
	m_Done(false)
{
	m_Lock = SDL_CreateMutex();
}

TraceMaster::~TraceMaster()
{
	SDL_DestroyMutex(m_Lock);
}

void TraceMaster::Halt()
{
	m_Halt = true;
	m_Done = true;
}

void TraceMaster::WaitForDone()
{
	// XXX make better
	while (!m_Done)
		sleep(1);
}

// simple tracer
TraceMasterSimple::TraceMasterSimple(RenderSurface &surface)
:	TraceMaster(surface),
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
	if (IsHalted() || m_IssuedLast)
		return -1;

	RenderSurface &surface = GetSurface();

	Lock();
	
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

	Unlock();

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

int TraceMasterSimple::ReturnWorkUnit(TraceWorkUnit &unit)
{
	colorf *c = unit.result;

	Lock();

//	std::cout << "return " << unit.startx << " " << unit.starty << " " << unit.endx << " " << unit.endy << std::endl;
	for (int x = unit.startx; x <= unit.endx; x++) {
		for (int y = unit.starty; y <= unit.endy; y++) {
			GetSurface().SetXY(x, y, *c);
			c++;
		}
	}

	m_PendingCount--;
	if (m_IssuedLast && m_PendingCount == 0)
		SetDone();

	Unlock();

	return 0;
}

// random Tracer
TraceMasterRandom::TraceMasterRandom(RenderSurface &surface)
:	TraceMaster(surface),
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
	if (IsHalted())
		return -1;

	RenderSurface &surface = GetSurface();

	if (m_Count == 0)
		return -1;

	Lock();

	for (;;) {
		uint x = rand() % (surface.Width() / WORKUNITSIZE);
		uint y = rand() % (surface.Height() / WORKUNITSIZE);

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

	Unlock();

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

	Lock();

//	std::cout << "return " << unit.startx << " " << unit.starty << " " << unit.endx << " " << unit.endy << std::endl;
	for (int x = unit.startx; x <= unit.endx; x++) {
		for (int y = unit.starty; y <= unit.endy; y++) {
			GetSurface().SetXY(x, y, *c);
			c++;
		}
	}

	m_PendingCount--;
	if (m_Count == 0 && m_PendingCount == 0)
		SetDone();

	Unlock();

	return 0;
}


