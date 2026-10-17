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
#include <cpp.h>
#include <memory>
#include <vector>

// vertex, normal, tangent and uv pools shared by the meshes loaded from one
// file. meshes keep a reference to their geometry, so it is never copied.
class geometry {
  public:
    geometry() = default;
    DISALLOW_COPY_ASSIGN_AND_MOVE(geometry);

    void add_vert(const vertex &v);
    void add_normal_vert(const vertex &v);
    void add_tangent_vert(const vertex &v);
    void add_uv_vert(const vertex &v);
    void add_mesh(std::unique_ptr<mesh> m);

    std::unique_ptr<mesh> create_mesh();

    const std::vector<std::unique_ptr<mesh>> &meshes() const { return meshes_; }
    const vertex_list &vertices() const { return vertices_; }
    const vertex_list &normals() const { return normals_; }
    const vertex_list &tangents() const { return tangents_; }
    const vertex_list &uvs() const { return uvs_; }

    void dump() const;

  private:
    std::vector<std::unique_ptr<mesh>> meshes_;

    vertex_list vertices_;
    vertex_list normals_;
    vertex_list tangents_;
    vertex_list uvs_;
};
