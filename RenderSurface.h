#ifndef __RENDER_SURFACE_H
#define __RENDER_SURFACE_H

#include <Color.h>
#include <string>

class RenderSurface {
public:
	RenderSurface(int width, int height);
	virtual ~RenderSurface();

//	int OpenOutFile(const std::string &name);

	int Width() const { return m_Width; }
	int Height() const { return m_Height; }

	virtual void SetXY(int x, int y, colorf color) = 0;
	virtual void SetXYRun(int x, int y, int count, const colorf *color) = 0;
//	colorf GetXY(int x, int y) const { return m_Buffer[y * m_Width + x]; }

#if 0
	class RenderSurfaceNotifyReceiver {
	public:
		virtual ~RenderSurfaceNotifyReceiver() {}
		virtual void RenderNotify(int width, int height) = 0;
	};

	void SetNotification(RenderSurfaceNotifyReceiver *notify);
	
	int WriteTGAFile(std::string filename);
#endif

private:
	int m_Width;
	int m_Height;
//	colorf *m_Buffer;
//	RenderSurfaceNotifyReceiver *m_Notify;
//	FILE *m_fp;
};

class RenderSurfaceFile : public RenderSurface {
public:
	RenderSurfaceFile(int width, int height);
	virtual ~RenderSurfaceFile();

	int OpenOutFile(const std::string &name);

	virtual void SetXY(int x, int y, colorf color);
	virtual void SetXYRun(int x, int y, int count, const colorf *color);

private:

	FILE *m_fp;
};

#endif
