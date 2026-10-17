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
#include <cmath>

#include <drawables/sphere.h>

using Libvec::Vector3d;

Sphere::Sphere(const Libvec::Vector3d &pos, double radius) : m_Center(pos), m_Radius(radius) {}

std::optional<Hit> Sphere::Intersect(const Ray &ray) const {
    double radius2 = m_Radius * m_Radius;

    Vector3d oc = m_Center - ray.origin;
    double l2oc = oc.LengthSquared();
    const bool inside = l2oc < radius2;

    double tca = Dot(oc, ray.dir);
    if (!inside && tca < 0) {
        // outside and pointing away from the sphere
        return std::nullopt;
    }

    double l2hc = (radius2 - l2oc) / ray.dir.LengthSquared() + (tca * tca);
    if (l2hc <= 0) {
        return std::nullopt;
    }

    // from inside the near root is behind the origin; take the far one
    Hit hit;
    hit.t = inside ? tca + std::sqrt(l2hc) : tca - std::sqrt(l2hc);
    hit.pos = ray.origin + ray.dir * hit.t;
    hit.normal = inside ? m_Center - hit.pos : hit.pos - m_Center;
    hit.normal.Normalize();
    return hit;
}
