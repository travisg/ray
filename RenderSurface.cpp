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
	m_file(NULL)
{
}

RenderSurfaceFile::~RenderSurfaceFile()
{
	if (m_file) {
		m_file->Close();
		delete m_file;
	}
}

int RenderSurfaceFile::OpenOutFile(const std::string &name)
{
	if (m_file) {
		m_file->Close();
		delete m_file;
	}

	m_file = new RayFile();
	return m_file->OpenWrite(name, Width(), Height());
}

void RenderSurfaceFile::SetXY(int x, int y, colorf color)
{
	if (m_file)
		m_file->SetXY(x, y, color);
}

void RenderSurfaceFile::SetXYRun(int x, int y, int count, const colorf *color)
{
	if (m_file)
		m_file->SetXYRun(x, y, count, color);
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
