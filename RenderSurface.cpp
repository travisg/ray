#include <cstdio>
#include <iostream>
#include <RenderSurface.h>

struct OutHeader {
	uint32_t magic;
	uint32_t width;
	uint32_t height;
};

enum RayDataType {
	TYPE_NULL = 0,
	TYPE_PIXEL,
	TYPE_EOF,
};

struct RayDataPixel {
	uint32_t x;
	uint32_t y;
	float r, g, b;
};

RenderSurface::RenderSurface(int width, int height)
:	m_Width(width),
	m_Height(height),
	m_Buffer(0),
	m_Notify(0),
	m_fp(0)
{
	m_Buffer = new colorf[width * height];
}

RenderSurface::~RenderSurface()
{
	delete[] m_Buffer;

	if (m_fp) {
		// write EOF token
		uint32_t type = TYPE_EOF;
		fwrite(&type, sizeof(type), 1, m_fp);

		fclose(m_fp);
	}
}

int RenderSurface::OpenOutFile(const std::string &name)
{
	m_fp = fopen(name.c_str(), "wb+");
	if (!m_fp)
		return -1;

	OutHeader header;

	header.magic = 'RAY1';
	header.width = m_Width;
	header.height = m_Height;

	fwrite(&header, sizeof(header), 1, m_fp);

	return 0;
}

void RenderSurface::SetXY(int x, int y, colorf color)
{
	m_Buffer[y * m_Width + x] = color;
	if (m_Notify)
		m_Notify->RenderNotify(x, y);

	// write a pixel packet
	if (m_fp) {
		uint32_t type = TYPE_PIXEL;

		fwrite(&type, sizeof(type), 1, m_fp);

		RayDataPixel pixel;
		pixel.x = x;
		pixel.y = y;
		pixel.r = color.r;
		pixel.g = color.g;
		pixel.b = color.b;
		fwrite(&pixel, sizeof(pixel), 1, m_fp);
	}
}

void RenderSurface::SetNotification(RenderSurfaceNotifyReceiver *notify)
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

