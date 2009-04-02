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

