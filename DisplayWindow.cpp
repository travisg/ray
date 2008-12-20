#include "DisplayWindow.h"
#include "RenderSurface.h"
#include <iostream>
#include <boost/bind.hpp>
#include <sdl.h>

DisplayWindow::DisplayWindow(int width, int height)
:	m_Width(width),
	m_Height(height),
	m_Surface(0),
	m_Screen(0),
	m_Dirty(false)
{
}

DisplayWindow::~DisplayWindow()
{
}

void DisplayWindow::Tick()
{
	if (m_Dirty) {
		SDL_UpdateRect(m_Screen, 0, 0, 0, 0);
		m_Dirty = false;
	}
}

static Uint32 TimerTick(Uint32 interval, void *param)
{
	DisplayWindow *dw = (DisplayWindow *)param;
	dw->Tick();

	return interval;
}

int DisplayWindow::CreateWindow()
{

	m_Screen = SDL_SetVideoMode(m_Surface->Width(), m_Surface->Height(), 32, SDL_SWSURFACE);
	SDL_UpdateRect(m_Screen, 0,0,0,0); // Update entire surface

	SDL_WM_SetCaption("Ray","ray");

	SDL_AddTimer(100, &TimerTick, this);

	return 0;
}

void DisplayWindow::SetRenderSurface(RenderSurface &surface)
{
	m_Surface = &surface;

	// set the callback
	surface.SetNotification(this);
}

void DisplayWindow::RenderNotify(int x, int y, colorf color)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.h = 1;
	rect.w = 1;
	color32 c32 = color;
	SDL_FillRect(m_Screen, &rect, c32);

	m_Dirty = true;
}

