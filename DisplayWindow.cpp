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
	m_SrcSurface(0),
	m_Dirty(false)
{
}

DisplayWindow::~DisplayWindow()
{
	SDL_FreeSurface(m_SrcSurface);
}

static Uint32 TimerTick(Uint32 interval, void *param)
{
	DisplayWindow *dw = (DisplayWindow *)param;
	dw->Tick();

	return interval;
}

int DisplayWindow::CreateWindow()
{

	m_Screen = SDL_SetVideoMode(m_Width, m_Height, 32, SDL_SWSURFACE);
	SDL_UpdateRect(m_Screen, 0,0,0,0); // Update entire surface

	SDL_WM_SetCaption("Ray","ray");

	SDL_AddTimer(100, &TimerTick, this);

	m_SrcSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,  
		m_Surface->Width(), m_Surface->Height(), 32, 
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	return 0;
}

void DisplayWindow::SetRenderSurface(RenderSurface &surface)
{
	m_Surface = &surface;

	// set the callback
	surface.SetNotification(this);
}

void DisplayWindow::RenderNotify(int x, int y)
{
#if 0
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.h = 1;
	rect.w = 1;
	color32 c32 = m_Surface->GetXY(x, y);
	SDL_FillRect(m_Screen, &rect, c32);
#endif
	m_Dirty = true;
}

void DisplayWindow::SurfaceBlit()
{
	if (m_Surface->Width() == m_Width && m_Surface->Height() == m_Height) {
		// 1:1 blit
		SDL_LockSurface(m_Screen);
		for (int x = 0; x < m_Width; x++) {
			for (int y = 0; y < m_Height; y++) {
				color32 c32 = m_Surface->GetXY(x, y);
				((uint32_t *)m_Screen->pixels)[y * m_Width + x] = c32;
			}
		}
		SDL_UnlockSurface(m_Screen);
	} else {
		SDL_LockSurface(m_SrcSurface);
		for (int x = 0; x < m_Surface->Width(); x++) {
			for (int y = 0; y < m_Surface->Height(); y++) {
				color32 c32 = m_Surface->GetXY(x, y);
				((uint32_t *)m_SrcSurface->pixels)[y * m_Surface->Width() + x] = c32;							
			}
		}
		SDL_UnlockSurface(m_SrcSurface);

		// scale between them
		SDL_Rect srcrect = { 0, 0, m_Surface->Width(), m_Surface->Height() };
		SDL_Rect dstrect = { 0, 0, m_Width, m_Height };
		SDL_SoftStretch(m_SrcSurface, &srcrect, m_Screen, &dstrect); 

		printf("src %d %d dest %d %d\n", m_Surface->Width(), m_Surface->Height(), m_Width, m_Height);
	}
}

void DisplayWindow::Tick()
{
	if (m_Dirty) {
		SurfaceBlit();
		SDL_UpdateRect(m_Screen, 0, 0, 0, 0);
		m_Dirty = false;
	}
}

