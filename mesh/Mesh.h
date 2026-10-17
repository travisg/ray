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

// A named group of surfaces indexing into vertex lists owned by the Geometry
// that created the mesh.
class Mesh {
  public:
    Mesh(const VertexList &posVerts, const VertexList &normalVerts, const VertexList &tangentVerts,
         const VertexList &uvVerts);
    ~Mesh();

    void AddSurface(Surface *s);

    void SetName(const std::string &name);
    const std::string &GetName() const { return m_Name; }

    void Dump() const;

    const SurfaceList &GetSurfaceList() const { return m_Surfaces; }

    const VertexList &GetVertexList() const { return m_Verts; }
    const VertexList &GetNormalVertexList() const { return m_NormalVerts; }
    const VertexList &GetTangentVertexList() const { return m_TangentVerts; }
    const VertexList &GetUVVertexList() const { return m_UVVerts; }

  private:
    std::string m_Name;

    const VertexList &m_Verts;
    const VertexList &m_NormalVerts;
    const VertexList &m_TangentVerts;
    const VertexList &m_UVVerts;

    SurfaceList m_Surfaces;
};

using MeshList = std::vector<Mesh *>;
