#ifndef __DRAWABLE_H
#define __DRAWABLE_H

#include <libmath/Vector3.h>
#include <Shader.h>

class Ray;

class Drawable {
public:
	Drawable();
	virtual ~Drawable();

	virtual bool Intersect(const Ray &ray) const = 0;
	virtual bool Intersect(const Ray &ray, Math::Vector3f &pos, Math::Vector3f &normal) const = 0;

	void SetShader(ShaderPtr s) { m_Shader = s; }
	ShaderPtr GetShader() const { return m_Shader; }

private:
	ShaderPtr m_Shader;
};


#endif

