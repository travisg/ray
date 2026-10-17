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

namespace Libvec {

template <typename T> class Vector3 {
  public:
    constexpr Vector3() = default;
    // Implicit on purpose: callers write `Vector3d v = 0;`.
    constexpr Vector3(T v) : x(v), y(v), z(v) {}
    constexpr Vector3(T x1, T y1, T z1) : x(x1), y(y1), z(z1) {}

    constexpr T getx() const { return x; }
    constexpr T gety() const { return y; }
    constexpr T getz() const { return z; }

    constexpr Vector3 operator+(const Vector3 &v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }
    constexpr Vector3 operator-(const Vector3 &v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }
    constexpr Vector3 operator-() const { return Vector3(-x, -y, -z); }
    constexpr Vector3 operator*(const Vector3 &v) const {
        return Vector3(x * v.x, y * v.y, z * v.z);
    }
    constexpr Vector3 operator*(T s) const { return Vector3(x * s, y * s, z * s); }
    constexpr Vector3 operator/(T s) const { return Vector3(x / s, y / s, z / s); }

    constexpr Vector3 &operator+=(const Vector3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    constexpr Vector3 &operator-=(const Vector3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    constexpr Vector3 &operator*=(const Vector3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    constexpr Vector3 &operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr Vector3 &operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    constexpr T LengthSquared() const { return x * x + y * y + z * z; }
    T Length() const { return std::sqrt(LengthSquared()); }
    constexpr T Dot(const Vector3 &v) const { return x * v.x + y * v.y + z * v.z; }

    // Scales to length n; a zero vector stays zero.
    void Normalize(T n = T(1)) {
        T l2 = LengthSquared();
        if (l2 == 0) {
            x = y = z = 0;
            return;
        }
        T s = n / std::sqrt(l2);
        x *= s;
        y *= s;
        z *= s;
    }

  private:
    T x = 0, y = 0, z = 0;
};

template <typename T> constexpr Vector3<T> operator*(T s, const Vector3<T> &v) {
    return v * s;
}

template <typename T> constexpr T Dot(const Vector3<T> &a, const Vector3<T> &b) {
    return a.Dot(b);
}

template <typename T> constexpr Vector3<T> Cross(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.gety() * b.getz() - a.getz() * b.gety(),
                      a.getz() * b.getx() - a.getx() * b.getz(),
                      a.getx() * b.gety() - a.gety() * b.getx());
}

template <typename T> std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
    return os << "[ " << v.getx() << " " << v.gety() << " " << v.getz() << " ]";
}

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;

} // namespace Libvec
