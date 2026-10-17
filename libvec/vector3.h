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

#include <cmath>
#include <ostream>

namespace libvec {

template <typename T> class vector3 {
  public:
    constexpr vector3() = default;
    // Implicit on purpose: callers write `vector3d v = 0;`.
    constexpr vector3(T v) : x(v), y(v), z(v) {}
    constexpr vector3(T x1, T y1, T z1) : x(x1), y(y1), z(z1) {}

    // axis 0, 1, 2
    constexpr T operator[](int axis) const { return axis == 0 ? x : axis == 1 ? y : z; }
    constexpr T &operator[](int axis) { return axis == 0 ? x : axis == 1 ? y : z; }

    constexpr vector3 operator+(const vector3 &v) const {
        return vector3(x + v.x, y + v.y, z + v.z);
    }
    constexpr vector3 operator-(const vector3 &v) const {
        return vector3(x - v.x, y - v.y, z - v.z);
    }
    constexpr vector3 operator-() const { return vector3(-x, -y, -z); }
    constexpr vector3 operator*(const vector3 &v) const {
        return vector3(x * v.x, y * v.y, z * v.z);
    }
    constexpr vector3 operator*(T s) const { return vector3(x * s, y * s, z * s); }
    constexpr vector3 operator/(T s) const { return vector3(x / s, y / s, z / s); }

    constexpr vector3 &operator+=(const vector3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    constexpr vector3 &operator-=(const vector3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    constexpr vector3 &operator*=(const vector3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    constexpr vector3 &operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr vector3 &operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    constexpr T length_squared() const { return x * x + y * y + z * z; }
    T length() const { return std::sqrt(length_squared()); }
    constexpr T dot(const vector3 &v) const { return x * v.x + y * v.y + z * v.z; }

    // Scales to length n; a zero vector stays zero.
    void normalize(T n = T(1)) {
        T l2 = length_squared();
        if (l2 == 0) {
            x = y = z = 0;
            return;
        }
        T s = n / std::sqrt(l2);
        x *= s;
        y *= s;
        z *= s;
    }

    // the coordinates are the public interface, as in any math library
    T x = 0, y = 0, z = 0;
};

template <typename T> constexpr vector3<T> operator*(T s, const vector3<T> &v) {
    return v * s;
}

template <typename T> constexpr T dot(const vector3<T> &a, const vector3<T> &b) {
    return a.dot(b);
}

template <typename T> constexpr vector3<T> cross(const vector3<T> &a, const vector3<T> &b) {
    return vector3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <typename T> std::ostream &operator<<(std::ostream &os, const vector3<T> &v) {
    return os << "[ " << v.x << " " << v.y << " " << v.z << " ]";
}

using vector3f = vector3<float>;
using vector3d = vector3<double>;

} // namespace libvec
