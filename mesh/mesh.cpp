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

mesh::mesh(const geometry &geom) : geometry_(geom) {}

void mesh::set_name(const std::string &name) {
    name_ = name;
}

void mesh::add_surface(surface s) {
    surfaces_.push_back(std::move(s));
}

const vertex_list &mesh::vertices() const {
    return geometry_.vertices();
}

const vertex_list &mesh::normals() const {
    return geometry_.normals();
}

const vertex_list &mesh::tangents() const {
    return geometry_.tangents();
}

const vertex_list &mesh::uvs() const {
    return geometry_.uvs();
}

void mesh::dump() const {
    std::cout << "\t\tMesh '" << name_ << "'";
    std::cout << ", " << surfaces_.size() << " surfaces" << std::endl;
}
