#ifndef __SPHERE_H
#define __SPHERE_H

#include <libmath/Vector3.h>
#include <Ray.h>
#include <drawables/Drawable.h>

class Sphere : public Drawable {
public:
	Sphere(const Math::Vector3d &pos, double radius);
	virtual ~Sphere();

	void SetPos(const Math::Vector3d &pos) { m_Center = pos; }
	void SetRadius(double rad) { m_Radius = rad; }

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Math::Vector3d &pos, Math::Vector3d &normal) const;

private:
	Math::Vector3d m_Center;
	double m_Radius;
};

#endif

