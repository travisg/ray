#ifndef __TRACER_H
#define __TRACER_H

#include <RenderSurface.h>
#include <libmath/Vector3.h>
#include <Ray.h>

class Scene;

class Tracer {
public:
	Tracer(RenderSurface &surface, Scene &scene);
	virtual ~Tracer();

	void Trace();

private:
	bool Cast(colorf &color, const Ray &ray);
	
	RenderSurface &m_Surface;

	Math::Vector3 m_Camera;
	Math::Vector3 m_Target;

	Scene &m_Scene;
};

#endif

