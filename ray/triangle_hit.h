/*
 * Copyright (c) 2026 Travis Geiselbrecht
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

#include <libvec/vector3.h>
#include <optional>
#include <ray/ray.h>

// Where a ray meets a triangle: the distance, the point, and the barycentric weights of v1
// and v2 there (v0's is 1 - b1 - b2).
struct triangle_hit {
    double t;
    libvec::vector3d pos;
    double b1;
    double b2;
};

// A point this far outside an edge, in barycentric units, still counts as inside. The hit
// point is computed from t and lands a rounding error off the plane, so without the slack a
// ray through an edge or a vertex shared by two triangles can be rejected by both.
inline constexpr double kEdgeSlop = 1e-9;

// Ray against the triangle v0 v1 v2 with the given face normal (any length, pointing out of
// the front face); back faces and parallel rays miss, as does anything at or behind the
// origin. http://graphics.stanford.edu/~mdfisher/Code/Engine/intersect.cpp.html
inline std::optional<triangle_hit> intersect_triangle(const ray &r, const libvec::vector3d &v0,
                                                      const libvec::vector3d &v1,
                                                      const libvec::vector3d &v2,
                                                      const libvec::vector3d &normal) {
    const double denom = dot(r.dir, normal);
    if (denom >= 0.0) {
        return std::nullopt;
    }
    // P = O + tV lies on the plane: ((P - v0) . N) = 0
    const double t = dot(v0 - r.origin, normal) / denom;
    if (t <= 0.0) {
        return std::nullopt;
    }
    const libvec::vector3d pos = r.origin + r.dir * t;

    const libvec::vector3d u = v1 - v0;
    const libvec::vector3d v = v2 - v0;
    const libvec::vector3d w = pos - v0;
    const double uu = dot(u, u);
    const double uv = dot(u, v);
    const double vv = dot(v, v);
    const double wu = dot(w, u);
    const double wv = dot(w, v);
    const double d = 1.0 / (uv * uv - uu * vv);

    const double b1 = (uv * wv - vv * wu) * d;
    if (b1 < -kEdgeSlop || b1 > 1.0 + kEdgeSlop) {
        return std::nullopt;
    }
    const double b2 = (uv * wu - uu * wv) * d;
    if (b2 < -kEdgeSlop || b1 + b2 > 1.0 + kEdgeSlop) {
        return std::nullopt;
    }
    return triangle_hit{ t, pos, b1, b2 };
}
