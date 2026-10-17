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

using libvec::vector3d;

sphere::sphere(const libvec::vector3d &pos, double radius) : center_(pos), radius_(radius) {}

std::optional<hit> sphere::intersect(const ray &r) const {
    double radius2 = radius_ * radius_;

    vector3d oc = center_ - r.origin;
    double l2oc = oc.length_squared();
    const bool inside = l2oc < radius2;

    double tca = dot(oc, r.dir);
    if (!inside && tca < 0) {
        // outside and pointing away from the sphere
        return std::nullopt;
    }

    double l2hc = (radius2 - l2oc) / r.dir.length_squared() + (tca * tca);
    if (l2hc <= 0) {
        return std::nullopt;
    }

    // from inside the near root is behind the origin; take the far one
    hit h;
    h.t = inside ? tca + std::sqrt(l2hc) : tca - std::sqrt(l2hc);
    h.pos = r.origin + r.dir * h.t;
    h.normal = inside ? center_ - h.pos : h.pos - center_;
    h.normal.normalize();
    return h;
}
