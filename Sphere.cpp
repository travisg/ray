#include <Sphere.h>

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

