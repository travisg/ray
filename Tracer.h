#ifndef __TRACER_H
#define __TRACER_H

#include <RenderSurface.h>
#include <libmath/Vector3.h>
#include <Ray.h>
#include <TraceMaster.h>

class Scene;

class Tracer {
public:
	Tracer(RenderSurface &surface, Scene &scene, TraceMaster &master);
	virtual ~Tracer();

	void Trace();

private:
	bool Cast(colorf &color, const Ray &ray);
	
	RenderSurface &m_Surface;

	Math::Vector3d m_Camera;
	Math::Vector3d m_Target;

	Scene &m_Scene;
	TraceMaster &m_Master;
};

#endif

