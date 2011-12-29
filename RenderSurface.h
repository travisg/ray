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

class RayFile;

class RenderSurfaceFile : public RenderSurface {
public:
	RenderSurfaceFile(int width, int height);
	virtual ~RenderSurfaceFile();

	int OpenOutFile(const std::string &name);

	virtual void SetXY(int x, int y, colorf color);
	virtual void SetXYRun(int x, int y, int count, const colorf *color);

private:
	RayFile *m_file;
};

#endif
