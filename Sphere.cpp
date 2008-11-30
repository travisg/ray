#include <cstdlib>
#include <math.h>

#include <Sphere.h>
#include <libmath/Math.h>

using Math::Vector3;

Sphere::Sphere()
{
}

Sphere::Sphere(const Math::Vector3 &pos, float radius)
:	m_Center(pos),
	m_Radius(radius)
{
}

Sphere::~Sphere()
{
}

bool Sphere::Intersect(const Ray &ray) const
{
	float radius2 = m_Radius * m_Radius;

	Vector3 oc = m_Center - ray.origin;
	float tca = Dot(oc, ray.dir);
	if (tca < 0) {
		// points away from the sphere
		return false;
	}

	float l2oc = oc.LengthSquared();
	float l2hc = (radius2 - l2oc) / ray.dir.LengthSquared() + (tca * tca);
	if (l2hc > 0) {
		return true;
	}

	return false;
}

bool Sphere::Intersect(const Ray &ray, Vector3 &pos, Vector3 &normal) const
{
	float radius2 = m_Radius * m_Radius;

	Vector3 oc = m_Center - ray.origin;
	float tca = Dot(oc, ray.dir);
	if (tca < 0) {
		// points away from the sphere
		return false;
	}

	float l2oc = oc.LengthSquared();
	float l2hc = (radius2 - l2oc) / ray.dir.LengthSquared() + (tca * tca);
	if (l2hc > 0) {
		float t = tca - sqrtf(l2hc);

		// calculate position
		pos = ray.origin + ray.dir * t;

		// radius ray
		normal = pos - m_Center;
		normal.Normalize();

		return true;
	}

	return false;
}

