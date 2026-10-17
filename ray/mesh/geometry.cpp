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
#include <iostream>
#include <ray/mesh/geometry.h>
#include <utility>

void geometry::add_mesh(std::unique_ptr<mesh> m) {
    meshes_.push_back(std::move(m));
}

void geometry::add_vert(const vertex &v) {
    vertices_.push_back(v);
}

void geometry::add_normal_vert(const vertex &v) {
    normals_.push_back(v);
}

void geometry::add_tangent_vert(const vertex &v) {
    tangents_.push_back(v);
}

void geometry::add_uv_vert(const vertex &v) {
    uvs_.push_back(v);
}

std::unique_ptr<mesh> geometry::create_mesh() {
    return std::make_unique<mesh>(*this);
}

void geometry::dump() const {
    std::cout << "geometry at " << (void *)this << std::endl;

    std::cout << "\t" << meshes_.size() << " meshes" << std::endl;
    std::cout << "\t" << vertices_.size() << " vertices" << std::endl;
    std::cout << "\t" << normals_.size() << " normal vertices" << std::endl;
    std::cout << "\t" << tangents_.size() << " tangent vertices" << std::endl;
    std::cout << "\t" << uvs_.size() << " uv vertices" << std::endl;

    for (const auto &m : meshes_) {
        m->dump();
    }
}
