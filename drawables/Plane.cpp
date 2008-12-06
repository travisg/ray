#include <iostream>

#include <Ray.h>
#include "Plane.h"

using Math::Vector3d;

Plane::Plane(const Ray &normal)
:	m_Normal(normal)
{
	m_Normal.dir.Normalize();
}


Plane::~Plane()
{
}

bool Plane::Intersect(const Ray &ray) const
{
//	std::cout << ray.dir << std::endl;
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
//	std::cout << t << std::endl;

	pos = ray.origin + ray.dir * t;
	normal = m_Normal.dir;

	return true;
}

