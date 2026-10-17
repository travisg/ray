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

#include <algorithm>
#include <libvec/vector3.h>
#include <limits>
#include <optional>
#include <ray/ray.h>

// Axis-aligned box; starts empty (min above max) so expand() builds it up.
struct aabb {
    libvec::vector3d min{ std::numeric_limits<double>::infinity() };
    libvec::vector3d max{ -std::numeric_limits<double>::infinity() };

    bool empty() const { return min.x > max.x; }

    void expand(const libvec::vector3d &p) {
        for (int a = 0; a < 3; a++) {
            min[a] = std::min(min[a], p[a]);
            max[a] = std::max(max[a], p[a]);
        }
    }
    void expand(const aabb &b) {
        expand(b.min);
        expand(b.max);
    }

    libvec::vector3d centroid() const { return (min + max) * 0.5; }
    libvec::vector3d extent() const { return max - min; }
    int longest_axis() const {
        const libvec::vector3d e = extent();
        return e.x >= e.y && e.x >= e.z ? 0 : e.y >= e.z ? 1 : 2;
    }

    // Distance along r to where it enters the box, 0 if it starts inside, nullopt if it misses
    // or only enters beyond t_max. inv_dir is 1 / r.dir per axis, infinities included: a zero
    // direction component gives +-inf products that fall out of the min/max, and a NaN from an
    // origin exactly on a slab is ignored by std::min/max, which keeps the box.
    std::optional<double> intersect(const ray &r, const libvec::vector3d &inv_dir,
                                    double t_max) const {
        double t0 = 0;
        double t1 = t_max;
        for (int a = 0; a < 3; a++) {
            double tnear = (min[a] - r.origin[a]) * inv_dir[a];
            double tfar = (max[a] - r.origin[a]) * inv_dir[a];
            if (inv_dir[a] < 0) {
                std::swap(tnear, tfar);
            }
            t0 = std::max(t0, tnear);
            t1 = std::min(t1, tfar);
            if (t0 > t1) {
                return std::nullopt;
            }
        }
        return t0;
    }
};
