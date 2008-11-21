#include <cstdlib>
#include <math.h>

#include <Sphere.h>
#include <math/Math.h>

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

bool Sphere::Intersect(const Vector3 &origin, const Vector3 &ray)
{
	float radius2 = m_Radius * m_Radius;

	Vector3 oc = m_Center - origin;
	float tca = Dot(oc, ray);
	if (tca < 0) {
		// points away from the sphere
		return false;
	}

	float l2oc = oc.LengthSquared();
	float l2hc = (radius2 - l2oc) / ray.LengthSquared() + (tca * tca);
	if (l2hc > 0) {
		return true;
	}

	return false;
}

bool Sphere::Intersect(const Vector3 &origin, const Vector3 &ray, Vector3 &pos, Vector3 &normal)
{
	float radius2 = m_Radius * m_Radius;

	Vector3 oc = m_Center - origin;
	float tca = Dot(oc, ray);
	if (tca < 0) {
		// points away from the sphere
		return false;
	}

	float l2oc = oc.LengthSquared();
	float l2hc = (radius2 - l2oc) / ray.LengthSquared() + (tca * tca);
	if (l2hc > 0) {
		float t = tca - sqrtf(l2hc);

		// calculate position
		pos = origin + ray * t;

		// radius ray
		normal = pos - m_Center;
		normal.Normalize();

		return true;
	}

	return false;
}

