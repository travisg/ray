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

#include "mesh.h"
#include <memory>
#include <vector>

// Vertex, normal, tangent and uv pools shared by the meshes loaded from one
// file. Meshes keep a reference to their geometry, so it is never copied.
class Geometry {
  public:
    Geometry() = default;
    Geometry(const Geometry &) = delete;
    Geometry &operator=(const Geometry &) = delete;

    void AddVert(const Vertex &v);
    void AddNormalVert(const Vertex &v);
    void AddTangentVert(const Vertex &v);
    void AddUVVert(const Vertex &v);
    void AddMesh(std::unique_ptr<Mesh> m);

    std::unique_ptr<Mesh> CreateMesh();

    const std::vector<std::unique_ptr<Mesh>> &Meshes() const { return m_Meshes; }
    const VertexList &Vertices() const { return m_Verts; }
    const VertexList &Normals() const { return m_NormalVerts; }
    const VertexList &Tangents() const { return m_TangentVerts; }
    const VertexList &UVs() const { return m_UVVerts; }

    void Dump() const;

  private:
    std::vector<std::unique_ptr<Mesh>> m_Meshes;

    VertexList m_Verts;
    VertexList m_NormalVerts;
    VertexList m_TangentVerts;
    VertexList m_UVVerts;
};
