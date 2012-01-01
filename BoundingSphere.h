/*
 * Copyright (c) 2008-2012 Travis Geiselbrecht
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
#ifndef __BOUNDING_SPHERE_H
#define __BOUNDING_SPHERE_H

#include <libvec/Vector3.h>
#include <Ray.h>

/* simple bounding sphere implementation */
struct BoundingSphere {
	bool DoesIntersect(const Ray &ray) const {
		double radius2 = radius * radius;

		Libvec::Vector3d oc = center - ray.origin;
		double tca = Dot(oc, ray.dir);
		if (tca < 0) {
			// points away from the sphere
			return false;
		}

		double l2oc = oc.LengthSquared();
		double l2hc = (radius2 - l2oc) / ray.dir.LengthSquared() + (tca * tca);
		if (l2hc > 0) {
			return true;
		}

		return false;
	}

	Libvec::Vector3d center;
	double radius;
};

#endif

