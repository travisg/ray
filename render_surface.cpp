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
#include <render_surface.h>

#include <ray_file.h>

RenderSurface::RenderSurface(int width, int height) : m_Width(width), m_Height(height) {}

RenderSurfaceFile::RenderSurfaceFile(int width, int height) : RenderSurface(width, height) {}

RenderSurfaceFile::~RenderSurfaceFile() = default;

int RenderSurfaceFile::OpenOutFile(const std::string &name) {
    m_file = std::make_unique<RayFile>();
    return m_file->OpenWrite(name, Width(), Height());
}

void RenderSurfaceFile::SetXY(int x, int y, colorf color) {
    if (m_file) {
        m_file->SetXY(x, y, color);
    }
}

void RenderSurfaceFile::SetXYRun(int x, int y, int count, const colorf *color) {
    if (m_file) {
        m_file->SetXYRun(x, y, count, color);
    }
}
