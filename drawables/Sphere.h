#ifndef __SPHERE_H
#define __SPHERE_H

#include <libvec/Vector3.h>
#include <Ray.h>
#include <drawables/Drawable.h>

class Sphere : public Drawable {
public:
	Sphere(const Libvec::Vector3d &pos, double radius);
	virtual ~Sphere();

	void SetPos(const Libvec::Vector3d &pos) { m_Center = pos; }
	void SetRadius(double rad) { m_Radius = rad; }

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Libvec::Vector3d &pos, Libvec::Vector3d &normal) const;

private:
	Libvec::Vector3d m_Center;
	double m_Radius;
};

#endif

