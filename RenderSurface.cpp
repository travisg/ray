#include <cstdio>
#include <iostream>
#include <RenderSurface.h>
#include <inttypes.h>

#include <RayFile.h>

RenderSurface::RenderSurface(int width, int height)
:	m_Width(width),
	m_Height(height)
{
}

RenderSurface::~RenderSurface()
{
}

RenderSurfaceFile::RenderSurfaceFile(int width, int height)
:	RenderSurface(width, height),
	m_fp(NULL)
{
}

RenderSurfaceFile::~RenderSurfaceFile()
{
	if (m_fp) {
		// write EOF token
		uint32_t type = TYPE_EOF;
		fwrite(&type, sizeof(type), 1, m_fp);

		fclose(m_fp);
	}
}

int RenderSurfaceFile::OpenOutFile(const std::string &name)
{
	m_fp = fopen(name.c_str(), "wb+");
	if (!m_fp)
		return -1;

	RayHeader header;

	header.magic = RAY_HEADER_MAGIC;
	header.width = Width();
	header.height = Height();

	fwrite(&header, sizeof(header), 1, m_fp);

	return 0;
}

void RenderSurfaceFile::SetXY(int x, int y, colorf color)
{
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

void RenderSurfaceFile::SetXYRun(int x, int y, int count, const colorf *color)
{
//	std::cout << __PRETTY_FUNCTION__ << " " << x << " " << y << " " << count << std::endl;
	if (m_fp) {
		uint32_t type = TYPE_PIXEL_RUN;

		fwrite(&type, sizeof(type), 1, m_fp);

		size_t buflen = sizeof(RayDataPixel) + sizeof(float) * 3 * count;
		char *buf = new char[buflen];
		RayDataPixelRun *run = (RayDataPixelRun *)buf;

		run->x = x;
		run->y = y;
		run->length = count;
		for (int i = 0; i < count; i++) {
			run->val[i*3] = color[i].r;
			run->val[i*3+1] = color[i].g;
			run->val[i*3+2] = color[i].b;
		}
		fwrite(run, buflen, 1, m_fp);

		delete[] buf;
	}
}

#if 0
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
#endif
