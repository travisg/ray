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
#include "triangle.h"
#include <ray.h>

using libvec::vector3d;

// http://graphics.stanford.edu/~mdfisher/Code/Engine/intersect.cpp.html

triangle::triangle(const libvec::vector3d &v0, const libvec::vector3d &v1,
                   const libvec::vector3d &v2) {
    v_[0] = v0;
    v_[1] = v1;
    v_[2] = v2;

    normal_ = cross(v1 - v0, v2 - v0);
    normal_.normalize();
}

std::optional<hit> triangle::intersect(const ray &r) const {
    double denom = dot(r.dir, normal_);
    if (denom >= 0.0) {
        // back face or parallel
        return std::nullopt;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = dot(v_[0] - r.origin, normal_) / denom;
    if (t <= 0.0) {
        // behind the origin
        return std::nullopt;
    }

    hit h;
    h.t = t;
    h.pos = r.origin + r.dir * t;
    h.normal = normal_;

    // barycentric coordinates of the hit within the triangle
    vector3d u = v_[1] - v_[0];
    vector3d v = v_[2] - v_[0];
    vector3d w = h.pos - v_[0];

    double uu = dot(u, u);
    double uv = dot(u, v);
    double vv = dot(v, v);
    double wu = dot(w, u);
    double wv = dot(w, v);
    double D = 1.0 / (uv * uv - uu * vv);

    double b1 = (uv * wv - vv * wu) * D;
    if (b1 < 0.0 || b1 > 1.0) {
        return std::nullopt;
    }
    double b2 = (uv * wu - uu * wv) * D;
    if (b2 < 0.0 || (b1 + b2) > 1.0) {
        return std::nullopt;
    }

    return h;
}
