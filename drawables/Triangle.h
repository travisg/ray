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
#ifndef __TRIANGLE_H
#define __TRIANGLE_H

#include <libvec/Vector3.h>
#include <Ray.h>

#include "Drawable.h"

class Triangle : public Drawable {
public:
	Triangle(const Libvec::Vector3d &v0, const Libvec::Vector3d &v1, const Libvec::Vector3d &v2);
	virtual ~Triangle();

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Libvec::Vector3d &pos, Libvec::Vector3d &normal) const;

private:
	Libvec::Vector3d m_v[3];
	Libvec::Vector3d m_normal;
};

#endif

