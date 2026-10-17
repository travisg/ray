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

#include <ostream>

struct colorf;

// Unclamped linear RGB.
struct colorf {
    double r = 0;
    double g = 0;
    double b = 0;

    colorf() = default;
    colorf(double _r, double _g, double _b) : r(_r), g(_g), b(_b) {}
    colorf(double c) : r(c), g(c), b(c) {}

    colorf operator*(double f) const { return colorf(r * f, g * f, b * f); }
    colorf operator*(const colorf &c) const { return colorf(r * c.r, g * c.g, b * c.b); }

    colorf &operator+=(const colorf &c) {
        r += c.r;
        g += c.g;
        b += c.b;
        return *this;
    }
    colorf &operator*=(double f) {
        r *= f;
        g *= f;
        b *= f;
        return *this;
    }
};

inline std::ostream &operator<<(std::ostream &os, const colorf &c) {
    os << "< " << c.r << " " << c.g << " " << c.b << " >";
    return os;
}
