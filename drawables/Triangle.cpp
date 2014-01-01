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
#include <iostream>

#include <Ray.h>
#include "Triangle.h"

using Libvec::Vector3d;

// http://graphics.stanford.edu/~mdfisher/Code/Engine/Intersect.cpp.html

Triangle::Triangle(const Libvec::Vector3d &v0, const Libvec::Vector3d &v1, const Libvec::Vector3d &v2)
{
    m_v[0] = v0;
    m_v[1] = v1;
    m_v[2] = v2;

    m_normal = Cross(v1 - v0,  v2 - v0);
    m_normal.Normalize();

    std::cout << "Triangle: normal " << v0 << " " << m_normal << std::endl;
}

Triangle::~Triangle()
{
}

bool Triangle::Intersect(const Ray &ray) const
{
//  std::cout << ray.dir << std::endl;
    Vector3d dir = ray.dir;
    dir.Normalize();

    if (Dot(dir, m_normal) > 0.0)
        return false;

//  std::cout << "intersected" << std::endl;

    return true;
}

bool Triangle::Intersect(const Ray &ray, Vector3d &pos, Vector3d &normal) const
{
    if (!Intersect(ray))
        return false;

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV

    double t = (Dot(m_v[0] - ray.origin, m_normal)) / (Dot(ray.dir, m_normal));

    // calculate the intersection point
    pos = ray.origin + ray.dir * t;
    normal = m_normal;

    // it intersects the plane the triangle is in, now calc the u/v coordinates and see if it's in the triangle
    float uu, uv, vv, wu, wv, D;

    Vector3d u = m_v[1] - m_v[0];
    Vector3d v = m_v[2] - m_v[0];

    uu = Dot(u, u);
    uv = Dot(u, v);
    vv = Dot(v, v);
    Vector3d w = pos - m_v[0];
    wu = Dot(w, u);
    wv = Dot(w, v);
    D = 1.0f / (uv * uv - uu * vv);

    // get and test parametric coords
    double s = (uv * wv - vv * wu) * D;
    if (s < 0.0f || s > 1.0f) {      // I is outside T
        return false;
    }
    t = (uv * wu - uu * wv) * D;
    if (t < 0.0f || (s + t) > 1.0f) { // I is outside T
        return false;
    }

//  std::cout << "s " << s << " t " << t << std::endl;

    return true;
}

