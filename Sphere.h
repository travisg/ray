#ifndef __SPHERE_H
#define __SPHERE_H

#include <libmath/Vector3.h>
#include <Ray.h>
#include <Drawable.h>

class Sphere : public Drawable {
public:
	Sphere();
	Sphere(const Math::Vector3d &pos, float radius);
	virtual ~Sphere();

	void SetPos(const Math::Vector3d &pos) { m_Center = pos; }
	void SetRadius(float rad) { m_Radius = rad; }

	virtual bool Intersect(const Ray &ray) const;
	virtual bool Intersect(const Ray &ray, Math::Vector3d &pos, Math::Vector3d &normal) const;

private:
	Math::Vector3d m_Center;
	float m_Radius;
};

#endif

