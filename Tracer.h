#ifndef __TRACER_H
#define __TRACER_H

#include <RenderSurface.h>
#include <math/Vector3.h>
#include <Ray.h>

class Tracer {
public:
	Tracer(RenderSurface &surface);
	virtual ~Tracer();

	void Trace();

private:
	bool Cast(colorf &color, const Ray &ray, bool collidetest);
	
	RenderSurface &m_Surface;

	Math::Vector3 m_Camera;
	Math::Vector3 m_Target;
};

#endif

