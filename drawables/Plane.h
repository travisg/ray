#ifndef __PLANE_H
#define __PLANE_H

#include <libmath/Vector3.h>
#include <Ray.h>

#include "Drawable.h"

class Plane : public Drawable {
public:
	Plane(const Ray &normal);
	virtual ~Plane();

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Math::Vector3d &pos, Math::Vector3d &normal) const;

private:
	Ray m_Normal;
};

#endif

