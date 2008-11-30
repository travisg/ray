#include <cstdio>
#include <iostream>
#include <RenderSurface.h>

RenderSurface::RenderSurface(int width, int height)
:	m_Width(width),
	m_Height(height),
	m_Buffer(0),
	m_Notify(0)
{
	m_Buffer = new colorf[width * height];
}

RenderSurface::~RenderSurface()
{
	delete[] m_Buffer;
}

void RenderSurface::SetXY(int x, int y, colorf color)
{
	m_Buffer[y * m_Width + x] = color;
	if (m_Notify)
		m_Notify(x, y, color);
}

void RenderSurface::SetNotification(RenderNotify notify)
{
	m_Notify = notify;
}

int RenderSurface::WriteTGAFile(std::string filename)
{
	FILE *fp;

	fp = fopen(filename.c_str(), "w+");
	if (!fp) {
		std::cerr << "error opening tga file " << filename << std::endl;
		return -1;
	}

	// TGA header
	unsigned char b;
	unsigned short s;

	b = 0; // identsize - none
	fwrite(&b, 1, 1, fp);
	b = 0; // colormaptype - none
	fwrite(&b, 1, 1, fp);
	b = 2; // imagetype - rgb
	fwrite(&b, 1, 1, fp);

	s = 0; // colormapstart - none
	fwrite(&s, 2, 1, fp);
	s = 0; // colormaplength - none
	fwrite(&s, 2, 1, fp);
	b = 0; // colormapbits - none
	fwrite(&b, 1, 1, fp);

	s = 0; // xstart
	fwrite(&s, 2, 1, fp);
	s = 0; // ystart
	fwrite(&s, 2, 1, fp);
	s = m_Width; // width
	fwrite(&s, 2, 1, fp);
	s = m_Height; // height
	fwrite(&s, 2, 1, fp);
	b = 24; // bits per pixel - 24
	fwrite(&b, 1, 1, fp);
	b = (1<<5); // descriptor bits - upper left
	fwrite(&b, 1, 1, fp);

	for (int i = 0; i < m_Width * m_Height; i++) {
		color32 c32 = m_Buffer[i];
		fwrite(&c32.b, 1, 1, fp);
		fwrite(&c32.g, 1, 1, fp);
		fwrite(&c32.r, 1, 1, fp);
	}

	fclose(fp);

	return 0;
}

