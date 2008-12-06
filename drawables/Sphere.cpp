#include <cstdlib>
#include <math.h>

#include <drawables/Sphere.h>
#include <libmath/Math.h>

using Math::Vector3d;

Sphere::Sphere(const Math::Vector3d &pos, double radius)
:	m_Center(pos),
	m_Radius(radius)
{
}

Sphere::~Sphere()
{
}

bool Sphere::Intersect(const Ray &ray) const
{
	double radius2 = m_Radius * m_Radius;

	Vector3d oc = m_Center - ray.origin;
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

bool Sphere::Intersect(const Ray &ray, Vector3d &pos, Vector3d &normal) const
{
	double radius2 = m_Radius * m_Radius;

	Vector3d oc = m_Center - ray.origin;
	double tca = Dot(oc, ray.dir);
	if (tca < 0) {
		// points away from the sphere
		return false;
	}

	double l2oc = oc.LengthSquared();
	double l2hc = (radius2 - l2oc) / ray.dir.LengthSquared() + (tca * tca);
	if (l2hc > 0) {
		double t = tca - sqrt(l2hc);

		// calculate position
		pos = ray.origin + ray.dir * t;

		// radius ray
		normal = pos - m_Center;
		normal.Normalize();

		return true;
	}

	return false;
}

