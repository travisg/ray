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
#include "Plane.h"
#include <Ray.h>

using Libvec::Vector3d;

Plane::Plane(const Ray &normal) : m_Normal(normal) {
    m_Normal.dir.Normalize();
}

std::optional<Hit> Plane::Intersect(const Ray &ray) const {
    double denom = Dot(ray.dir, m_Normal.dir);
    if (denom >= 0.0) {
        // back face or parallel
        return std::nullopt;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = Dot(m_Normal.origin - ray.origin, m_Normal.dir) / denom;
    if (t <= 0.0) {
        // behind the origin
        return std::nullopt;
    }

    Hit hit;
    hit.t = t;
    hit.pos = ray.origin + ray.dir * t;
    hit.normal = m_Normal.dir;
    return hit;
}
