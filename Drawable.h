#ifndef __DRAWABLE_H
#define __DRAWABLE_H

#include <math/Vector3.h>

class Ray;
class Shader;

class Drawable {
public:
	Drawable() : m_Shader(0) {}
	virtual ~Drawable() {}

	virtual bool Intersect(const Ray &ray) const = 0;
	virtual bool Intersect(const Ray &ray, Math::Vector3 &pos, Math::Vector3 &normal) const = 0;

	void SetShader(Shader *s) { m_Shader = s; }
	Shader *GetShader() const { return m_Shader; }

private:
	Shader *m_Shader;
};


#endif

