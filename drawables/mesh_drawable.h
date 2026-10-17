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

#include <bounding_sphere.h>
#include <drawables/drawable.h>
#include <libvec/vector3.h>
#include <memory>
#include <ray.h>

class geometry;

// triangle/quad mesh loaded from a geometry, with a bounding-sphere pre-test.
class mesh_drawable : public drawable {
  public:
    explicit mesh_drawable(std::unique_ptr<geometry> geom);
    ~mesh_drawable() override;

    // computes the bounding sphere; must run before intersect
    int prepare() override;

    std::optional<hit> intersect(const ray &r) const override;

  private:
    std::unique_ptr<geometry> geometry_;

    bounding_sphere bounding_sphere_;
};
