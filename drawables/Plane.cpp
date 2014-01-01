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
#include "Plane.h"

using Libvec::Vector3d;

Plane::Plane(const Ray &normal)
    :   m_Normal(normal)
{
    m_Normal.dir.Normalize();
}


Plane::~Plane()
{
}

bool Plane::Intersect(const Ray &ray) const
{
//  std::cout << ray.dir << std::endl;
    Vector3d dir = ray.dir;
    dir.Normalize();

    if (Dot(dir, m_Normal.dir) > 0.0)
        return false;

    return true;
}

bool Plane::Intersect(const Ray &ray, Vector3d &pos, Vector3d &normal) const
{
    if (!Intersect(ray))
        return false;

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV

    double t = (Dot(m_Normal.origin - ray.origin, m_Normal.dir)) / (Dot(ray.dir, m_Normal.dir));
//  std::cout << t << std::endl;

    pos = ray.origin + ray.dir * t;
    normal = m_Normal.dir;

    return true;
}

