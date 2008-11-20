#ifndef __RENDER_SURFACE_H
#define __RENDER_SURFACE_H

typedef unsigned int color32;

class RenderSurface {
public:
	RenderSurface(int width, int height);
	virtual ~RenderSurface();

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }

	void SetXY(int width, int height, color32 color);

	typedef void (*RenderNotify)(int x, int y, color32);
	void SetNotification(RenderNotify notify);

private:
	int m_Width;
	int m_Height;
	color32 *m_Buffer;
	RenderNotify m_Notify;
};


#endif
