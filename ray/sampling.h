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

#include <cmath>
#include <cstdint>
#include <libvec/vector3.h>
#include <numbers>

// The k-th point of the van der Corput sequence in the given base: k's digits reflected
// about the radix point, so 1, 2, 3, 4 in base 2 give 0.5, 0.25, 0.75, 0.125. Successive
// points land in the biggest gap left so far, which is what makes the sequence evenly
// spread for any count. Bases 2 and 3 together are the 2D Halton sequence.
inline double radical_inverse(uint32_t k, uint32_t base) {
    double result = 0;
    double digit = 1.0 / base;
    while (k > 0) {
        result += (k % base) * digit;
        k /= base;
        digit /= base;
    }
    return result;
}

// fractional part, for shifting a sequence point by an offset and wrapping it into [0, 1)
inline double fract(double v) {
    return v - std::floor(v);
}

// A grid over the unit square with at least n cells, as square as n allows: 64 is 8x8, 50 is
// 8x7. One random point per cell (stratified sampling) spreads n samples evenly, where n
// independent points would clump and leave gaps.
struct strata {
    int x;
    int y;
    int count() const { return x * y; }
};

inline strata stratify(int n) {
    const int x = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n))));
    return { x, (n + x - 1) / x };
}

// A direction on the hemisphere around the unit normal n, with probability proportional to
// the cosine of its angle to n (Malley's method: a uniform disc point lifted to the sphere).
// u1 and u2 are uniform in [0, 1). Under this density the diffuse estimator is just the
// average of what the rays see, times the albedo.
inline libvec::vector3d cosine_hemisphere(const libvec::vector3d &n, double u1, double u2) {
    const double r = std::sqrt(u1);
    const double phi = 2.0 * std::numbers::pi * u2;
    const double x = r * std::cos(phi);
    const double y = r * std::sin(phi);
    const double z = std::sqrt(std::max(0.0, 1.0 - u1));

    // a tangent frame: pick the helper axis least aligned with n
    const libvec::vector3d helper =
        std::fabs(n.x) < 0.9 ? libvec::vector3d(1, 0, 0) : libvec::vector3d(0, 1, 0);
    libvec::vector3d t = cross(helper, n);
    t.normalize();
    const libvec::vector3d b = cross(n, t);
    return t * x + b * y + n * z;
}
