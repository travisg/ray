#ifndef __DRAWABLE_H
#define __DRAWABLE_H

class Drawable {
public:
	Drawable() {}
	virtual ~Drawable() {}

	virtual bool Intersect(const Ray &ray) const = 0;
	virtual bool Intersect(const Ray &ray, Math::Vector3 &pos, Math::Vector3 &normal) const = 0;

private:
};


#endif

