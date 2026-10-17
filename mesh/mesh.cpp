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
#include "mesh.h"
#include "geometry.h"
#include <iostream>
#include <utility>

Mesh::Mesh(const Geometry &geom) : m_Geom(geom) {}

void Mesh::SetName(const std::string &name) {
    m_Name = name;
}

void Mesh::AddSurface(Surface s) {
    m_Surfaces.push_back(std::move(s));
}

const VertexList &Mesh::GetVertexList() const {
    return m_Geom.Vertices();
}

const VertexList &Mesh::GetNormalVertexList() const {
    return m_Geom.Normals();
}

const VertexList &Mesh::GetTangentVertexList() const {
    return m_Geom.Tangents();
}

const VertexList &Mesh::GetUVVertexList() const {
    return m_Geom.UVs();
}

void Mesh::Dump() const {
    std::cout << "\t\tMesh '" << m_Name << "'";
    std::cout << ", " << m_Surfaces.size() << " surfaces" << std::endl;
}
