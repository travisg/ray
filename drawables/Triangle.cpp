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
#include "Triangle.h"
#include <Ray.h>

using Libvec::Vector3d;

// http://graphics.stanford.edu/~mdfisher/Code/Engine/Intersect.cpp.html

Triangle::Triangle(const Libvec::Vector3d &v0, const Libvec::Vector3d &v1,
                   const Libvec::Vector3d &v2) {
    m_v[0] = v0;
    m_v[1] = v1;
    m_v[2] = v2;

    m_normal = Cross(v1 - v0, v2 - v0);
    m_normal.Normalize();
}

std::optional<Hit> Triangle::Intersect(const Ray &ray) const {
    double denom = Dot(ray.dir, m_normal);
    if (denom >= 0.0) {
        // back face or parallel
        return std::nullopt;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = Dot(m_v[0] - ray.origin, m_normal) / denom;
    if (t <= 0.0) {
        // behind the origin
        return std::nullopt;
    }

    Hit hit;
    hit.t = t;
    hit.pos = ray.origin + ray.dir * t;
    hit.normal = m_normal;

    // it intersects the plane the triangle is in, now calc the u/v coordinates and see if it's in
    // the triangle
    float uu, uv, vv, wu, wv, D;

    Vector3d u = m_v[1] - m_v[0];
    Vector3d v = m_v[2] - m_v[0];

    uu = Dot(u, u);
    uv = Dot(u, v);
    vv = Dot(v, v);
    Vector3d w = hit.pos - m_v[0];
    wu = Dot(w, u);
    wv = Dot(w, v);
    D = 1.0f / (uv * uv - uu * vv);

    // get and test parametric coords
    double b1 = (uv * wv - vv * wu) * D;
    if (b1 < 0.0f || b1 > 1.0f) {
        return std::nullopt;
    }
    double b2 = (uv * wu - uu * wv) * D;
    if (b2 < 0.0f || (b1 + b2) > 1.0f) {
        return std::nullopt;
    }

    return hit;
}
