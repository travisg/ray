#include <RenderSurface.h>

RenderSurface::RenderSurface(int width, int height)
:	m_Width(width),
	m_Height(height),
	m_Buffer(0),
	m_Notify(0)
{
	m_Buffer = new color32[width * height];
}

RenderSurface::~RenderSurface()
{
	delete[] m_Buffer;
}

void RenderSurface::SetXY(int x, int y, color32 color)
{
	m_Buffer[y * m_Width + x] = color;
	if (m_Notify)
		m_Notify(x, y, color);
}

void RenderSurface::SetNotification(RenderNotify notify)
{
	m_Notify = notify;
}

