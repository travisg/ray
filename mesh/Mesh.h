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
#pragma once

#include "Surface.h"
#include "Vertex.h"
#include <string>
#include <vector>

class Geometry;

// A named group of surfaces indexing into the vertex lists of the Geometry
// that owns the mesh.
class Mesh {
  public:
    explicit Mesh(const Geometry &geom);

    void AddSurface(Surface s);

    void SetName(const std::string &name);
    const std::string &GetName() const { return m_Name; }

    void Dump() const;

    const std::vector<Surface> &GetSurfaceList() const { return m_Surfaces; }

    const VertexList &GetVertexList() const;
    const VertexList &GetNormalVertexList() const;
    const VertexList &GetTangentVertexList() const;
    const VertexList &GetUVVertexList() const;

  private:
    const Geometry &m_Geom;
    std::string m_Name = "default";
    std::vector<Surface> m_Surfaces;
};
