#ifndef __PLANE_H
#define __PLANE_H

#include <libvec/Vector3.h>
#include <Ray.h>

#include "Drawable.h"

class Plane : public Drawable {
public:
	Plane(const Ray &normal);
	virtual ~Plane();

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Libvec::Vector3d &pos, Libvec::Vector3d &normal) const;

private:
	Ray m_Normal;
};

#endif

