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
#include "Geometry.h"
#include <iostream>
#include <utility>

void Geometry::AddMesh(std::unique_ptr<Mesh> m) {
    m_Meshes.push_back(std::move(m));
}

void Geometry::AddVert(const Vertex &v) {
    m_Verts.push_back(v);
}

void Geometry::AddNormalVert(const Vertex &v) {
    m_NormalVerts.push_back(v);
}

void Geometry::AddTangentVert(const Vertex &v) {
    m_TangentVerts.push_back(v);
}

void Geometry::AddUVVert(const Vertex &v) {
    m_UVVerts.push_back(v);
}

std::unique_ptr<Mesh> Geometry::CreateMesh() {
    return std::make_unique<Mesh>(*this);
}

void Geometry::Dump() const {
    std::cout << "Geometry at " << (void *)this << std::endl;

    std::cout << "\t" << m_Meshes.size() << " meshes" << std::endl;
    std::cout << "\t" << m_Verts.size() << " vertices" << std::endl;
    std::cout << "\t" << m_NormalVerts.size() << " normal vertices" << std::endl;
    std::cout << "\t" << m_TangentVerts.size() << " tangent vertices" << std::endl;
    std::cout << "\t" << m_UVVerts.size() << " uv vertices" << std::endl;

    for (const auto &m : m_Meshes) {
        m->Dump();
    }
}
