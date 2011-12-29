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
#ifndef __DISPLAY_WINDOW_H
#define __DISPLAY_WINDOW_H

#include <Color.h>
#include <RenderSurface.h>
#include <sdl.h>

class DisplayWindow : public RenderSurface::RenderSurfaceNotifyReceiver {
public:
	DisplayWindow(int width, int height);
	virtual ~DisplayWindow();

	int CreateWindow();
	
	void SetRenderSurface(RenderSurface &surface);

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }

	virtual void RenderNotify(int x, int y);

	void Tick();

private:
	void SurfaceBlit();

	int m_Width;
	int m_Height;
	RenderSurface *m_Surface;

	SDL_Surface *m_Screen;
	SDL_Surface *m_SrcSurface;
	SDL_mutex *m_Lock;
	volatile bool m_Dirty;
};

#endif

