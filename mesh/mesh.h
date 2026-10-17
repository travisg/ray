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

#include "surface.h"
#include "vertex.h"
#include <string>
#include <vector>

class geometry;

// A named group of surfaces indexing into the vertex lists of the geometry
// that owns the mesh.
class mesh {
  public:
    explicit mesh(const geometry &geom);

    void add_surface(surface s);

    void set_name(const std::string &name);
    const std::string &name() const { return name_; }

    void dump() const;

    const std::vector<surface> &surfaces() const { return surfaces_; }

    const vertex_list &vertices() const;
    const vertex_list &normals() const;
    const vertex_list &tangents() const;
    const vertex_list &uvs() const;

  private:
    const geometry &geometry_;
    std::string name_ = "default";
    std::vector<surface> surfaces_;
};
