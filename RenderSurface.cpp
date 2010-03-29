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
