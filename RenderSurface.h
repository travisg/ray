#ifndef __RENDER_SURFACE_H
#define __RENDER_SURFACE_H

#include <Color.h>
#include <string>

class RenderSurface {
public:
	RenderSurface(int width, int height);
	virtual ~RenderSurface();

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }

	void SetXY(int x, int y, colorf color);
	colorf GetXY(int x, int y) const { return m_Buffer[y * m_Width + x]; }

	class RenderSurfaceNotifyReceiver {
	public:
		virtual ~RenderSurfaceNotifyReceiver() {}
		virtual void RenderNotify(int width, int height) = 0;
	};

	void SetNotification(RenderSurfaceNotifyReceiver *notify);
	
	int WriteTGAFile(std::string filename);

private:
	int m_Width;
	int m_Height;
	colorf *m_Buffer;
	RenderSurfaceNotifyReceiver *m_Notify;
};

#endif
