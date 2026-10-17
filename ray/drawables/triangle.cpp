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
#include <ray/drawables/triangle.h>
#include <ray/ray.h>
#include <ray/triangle_hit.h>

using libvec::vector3d;

triangle::triangle(const libvec::vector3d &v0, const libvec::vector3d &v1,
                   const libvec::vector3d &v2) {
    v_[0] = v0;
    v_[1] = v1;
    v_[2] = v2;

    normal_ = cross(v1 - v0, v2 - v0);
    normal_.normalize();
}

std::optional<hit> triangle::intersect(const ray &r) const {
    std::optional<triangle_hit> th = intersect_triangle(r, v_[0], v_[1], v_[2], normal_);
    if (!th) {
        return std::nullopt;
    }
    hit h;
    h.t = th->t;
    h.pos = th->pos;
    h.normal = normal_;
    return h;
}
