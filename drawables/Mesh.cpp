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
#include <cstdlib>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <drawables/Mesh.h>
#include <libvec/Vec.h>
#include <BoundingSphere.h>
#include <mesh/Geometry.h>

using Libvec::Vector3d;

MeshDrawable::MeshDrawable(boost::shared_ptr<Geometry> g)
:	m_Geom(g)
{
}

MeshDrawable::~MeshDrawable()
{
}

int MeshDrawable::Prepare()
{
	std::cout << __PRETTY_FUNCTION__ << " " << (void *)this << std::endl;

	// try to set up the bounding sphere
	
	// guess the center by finding the average x, y, z for all the vertexes in the geometry
	Vector3d avg(0, 0, 0);
	int count = 0;
	for (std::vector<Vertex>::const_iterator i = m_Geom->VertexListIterator(); i != m_Geom->VertexListEnd(); i++) {
		avg += Vector3d((*i).x, (*i).y, (*i).z);
		count++;
	}
	avg /= count;

	std::cout << "average x y z = " << avg << std::endl;

	m_BSphere.center = avg;

	// find the fartest outlier
	double farthest = 0;
	for (std::vector<Vertex>::const_iterator i = m_Geom->VertexListIterator(); i != m_Geom->VertexListEnd(); i++) {
		Vector3d v((*i).x, (*i).y, (*i).z);

		double dist = (v - m_BSphere.center).Length();

		if (dist > farthest) {
			farthest = dist;
			std::cout << "found farthest at " << v << " distance " << dist << std::endl;
		}
	}

	m_BSphere.radius = farthest;

	return 0;
}

bool MeshDrawable::Intersect(const Ray &ray) const
{
	if (!m_BSphere.DoesIntersect(ray))
		return false;

//	std::cout << __PRETTY_FUNCTION__ << " potential intersect" << std::endl;

	return false;
}

bool TriIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Vector3d &v0, const Vector3d &v1, const Vector3d &v2)
{
//	std::cout << v0 << " " << v1 << " " << v2 << std::endl;

	normal = Cross(v1 - v0,  v2 - v0);

	// is the ray pointing at the surface?
	if (Dot(ray.dir, normal) > 0)
		return false;

	// t = ((P - P0) . N) / (V . N)
	// P = P0 + tV

	double t = (Dot(v0 - ray.origin, normal)) / (Dot(ray.dir, normal));

	// is the origin behind the start of the ray
	if (t < 0.0f)
		return false;

	// calculate the intersection point
	pos = ray.origin + ray.dir * t;

	// it intersects the plane the triangle is in, now calc the u/v coordinates and see if it's in the triangle
	double uu, uv, vv, wu, wv, D;

	Vector3d u = v1 - v0;
	Vector3d v = v2 - v0;

	uu = Dot(u, u);
	uv = Dot(u, v);
	vv = Dot(v, v);
	Vector3d w = pos - v0;
	wu = Dot(w, u);
	wv = Dot(w, v);
	D = (double)1.0f / (uv * uv - uu * vv);

	// get and test parametric coords
	double s = (uv * wv - vv * wu) * D;
	if (s < 0.0f || s > 1.0f)        // I is outside T
	{
		return false;
	}
	t = (uv * wu - uu * wv) * D;
	if (t < 0.0f || (s + t) > 1.0f)  // I is outside T
	{
		return false;
	}

	// normalize the normal before sending it back
	normal.Normalize();

//	std::cout << "s " << s << " t " << t << std::endl;

#if 0
	if ((ray.origin - pos).Length() < 0.10f) {
		std::cout << "ray cast very close to surface\n";
	}
	std::cout << "ray " << ray.origin << "\n";
	std::cout << "ray dir " << ray.dir << "\n";
	std::cout << "intersection " << pos << "\n";
	std::cout << "normal " << normal << "\n";
	std::cout << "dot with normal " << Dot(ray.dir, normal) << "\n";
	std::cout << "v0 " << v0 << "\n";
	std::cout << "v1 " << v1 << "\n";
	std::cout << "v2 " << v2 << "\n";
	std::cout << "light ray " << ray.light << std::endl;
#endif

	return true;
}

bool QuadIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Vector3d &v0, const Vector3d &v1, const Vector3d &v2, const Vector3d &v3)
{
//	std::cout << "quad" << v0 << " " << v1 << " " << v2 << " " << v3 << std::endl;
	if (TriIntersect(ray, pos, normal, v0, v1, v2)) {
		return true;
	}
	return TriIntersect(ray, pos, normal, v2, v3, v0);
}

bool SurfaceIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Mesh &mesh, const Surface &surface)
{
#if 0
	std::cout << surface.m_Indexes[0].posIndex << " ";
	std::cout << surface.m_Indexes[1].posIndex << " ";
	std::cout << surface.m_Indexes[2].posIndex;
	if (surface.m_Indexes.size() == 4)
		std::cout << " " << surface.m_Indexes[3].posIndex;
	std::cout << std::endl;
#endif

	// get the vertexes that make up this surface
	const VertexList &verts = mesh.GetVertexList();

	const Vertex *vp = &verts[surface.m_Indexes[0].posIndex];
	Vector3d v0(vp->x, vp->y, vp->z);
	vp = &verts[surface.m_Indexes[1].posIndex];
	Vector3d v1(vp->x, vp->y, vp->z);
	vp = &verts[surface.m_Indexes[2].posIndex];
	Vector3d v2(vp->x, vp->y, vp->z);
	if (surface.m_Indexes.size() == 3) {
		if (TriIntersect(ray, pos, normal, v0, v1, v2) == false)
			return false;
	} else {
		vp = &verts[surface.m_Indexes[3].posIndex];
		Vector3d v3(vp->x, vp->y, vp->z);

		if (QuadIntersect(ray, pos, normal, v0, v1, v2, v3) == false)
			return false;
	}

//	std::cout << "intersected" << std::endl;

	return true;
}

bool MeshDrawable::Intersect(const Ray &ray, Vector3d &pos, Vector3d &normal) const
{
	if (!m_BSphere.DoesIntersect(ray)) {
//		std::cout << "failed bounding box intersection" << std::endl;
		return false;
	}

	// potential collision
	const Mesh *meshmatch = 0;
	const Surface *surfmatch = 0;
	double matchdist = -1;

	// for every mesh, check every surface
	for (MeshListIteratorConst m = m_Geom->ListIterator(); m != m_Geom->ListEnd(); m++) {
		// for every surface in this mesh
		for (SurfaceListIteratorConst s = (*m)->GetSurfaceList().begin(); s != (*m)->GetSurfaceList().end(); s++) {
//			std::cout << (*s)->m_Indexes.size() << std::endl;

			if (SurfaceIntersect(ray, pos, normal, **m, **s) == false)
				continue;

			// surface intersected, lets see if it's the best match
			double distance = (pos - ray.origin).LengthSquared();
			if (matchdist < 0 || distance < matchdist) {
				meshmatch = *m;
				surfmatch = *s;
				matchdist = distance;
			}
		}
	}

	// rerun the closest match to the caster
	if (meshmatch) {
		SurfaceIntersect(ray, pos, normal, *meshmatch, *surfmatch);
		return true;
	}

	return false;
}

