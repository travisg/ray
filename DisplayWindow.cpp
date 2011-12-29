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
#include "DisplayWindow.h"
#include "RenderSurface.h"
#include <iostream>
#include <boost/bind.hpp>
#include <sys/time.h>
#include <time.h>
#include <sdl.h>

DisplayWindow::DisplayWindow(int width, int height)
:	m_Width(width),
	m_Height(height),
	m_Surface(0),
	m_Screen(0),
	m_SrcSurface(0),
	m_Dirty(false)
{
	m_Lock = SDL_CreateMutex();
}

DisplayWindow::~DisplayWindow()
{
	SDL_FreeSurface(m_SrcSurface);
	SDL_DestroyMutex(m_Lock);
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

	SDL_AddTimer(1000, &TimerTick, this);

	m_SrcSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,  
		m_Surface->Width(), m_Surface->Height(), 32, 
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	return 0;
}

void DisplayWindow::SetRenderSurface(RenderSurface &surface)
{
	m_Surface = &surface;

	// set the callback
//	surface.SetNotification(this);
}

void DisplayWindow::RenderNotify(int x, int y)
{
	SDL_LockMutex(m_Lock);

	SDL_LockSurface(m_SrcSurface);
	color32 c32 = m_Surface->GetXY(x, y);
	((uint32_t *)m_SrcSurface->pixels)[y * m_Surface->Width() + x] = c32;							
	SDL_UnlockSurface(m_SrcSurface);

	m_Dirty = true;

	SDL_UnlockMutex(m_Lock);
}

void DisplayWindow::SurfaceBlit()
{
	// scale between them
	SDL_Rect srcrect = { 0, 0, m_Surface->Width(), m_Surface->Height() };
	SDL_Rect dstrect = { 0, 0, m_Width, m_Height };
	SDL_LockSurface(m_SrcSurface);
	SDL_SoftStretch(m_SrcSurface, &srcrect, m_Screen, &dstrect); 
	SDL_UnlockSurface(m_SrcSurface);

//	printf("src %d %d dest %d %d\n", m_Surface->Width(), m_Surface->Height(), m_Width, m_Height);
}

void DisplayWindow::Tick()
{
	if (m_Dirty) {
		m_Dirty = false;
		SurfaceBlit();
		SDL_UpdateRect(m_Screen, 0, 0, 0, 0);
	}
}

