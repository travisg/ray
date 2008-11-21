#ifndef __SPHERE_H
#define __SPHERE_H

#include <math/Math.h>

class Sphere {
	Sphere();
	Sphere(const Math::Vector3 &pos, float radius);
	virtual ~Sphere();

	void SetPos(const Math::Vector3 &pos) { m_Center = pos; }
	void SetRadius(float rad) { m_Radius = rad; }

private:
	Math::Vector3 m_Center;
	float m_Radius;
}

#endif

