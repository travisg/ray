
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <vector>

#include <Scene.h>
#include <Ray.h>
#include <Tracer.h>
#include <drawables/Sphere.h>

#include <shaders/Shader.h>

using Math::Vector3d;

Tracer::Tracer(RenderSurface &surface, Scene &scene, TraceMaster &tracemaster)
:	m_Surface(surface),
	m_Scene(scene),
	m_Master(tracemaster),
	m_Depth(0)
{
}

Tracer::~Tracer()
{
}

bool Tracer::Cast(colorf &color, const Ray &ray)
{
	if (m_Depth == kMaxTraceDepth)
		return false;

	// tell the scene graph to find an object to intersect
	const Drawable *d = m_Scene.Intersect(ray);

	// render the closest one
	if (d) {
		Vector3d pos;
		Vector3d normal;
		d->Intersect(ray, pos, normal);

		boost::shared_ptr<Shader> s = d->GetShader();
		if (!s) {
			color = 0;
		} else {
			ShaderArgs args;

			args.scene = &m_Scene;
			args.tracer = this;
			args.d = d;
			args.ray = &ray;
			args.pos = pos;
			args.normal = normal;

			m_Depth++; // track the level of recursion
			color = s->Run(args);
			m_Depth--;
		}

		return true;
	}

	return false;
}

void Tracer::Trace()
{
	int width = m_Surface.Width();
	int height = m_Surface.Height();

//	std::cout << "Camera " << m_Camera << std::endl;
//	std::cout << "Target " << m_Target << std::endl;

	Math::Vector3d target = m_Scene.GetTarget();
	target -= m_Scene.GetCamera();
	target.Normalize();
	target += m_Scene.GetCamera();

	Vector3d lin = target - m_Scene.GetCamera();

//	std::cout << "Lin " << lin << std::endl;

//	int multisample = 1;
	float pixpitch = 2.0/width;
//	float subpitch = pixpitch / ((float)multisample + 1.0);

	TraceWorkUnit unit;
	while (m_Master.GetWorkUnit(unit) >= 0) {
		for (int y = unit.starty; y <= unit.endy; y++) {
			for (int x = unit.startx; x <= unit.endx; x++) {
				Vector3d wing(lin.gety(), -lin.getx(), 0);

		//			std::cout << "wing " << wing << std::endl;

				Vector3d head = Cross(lin, wing);

		//			std::cout << "head " << head << std::endl;

				wing.Normalize((x - width / 2.0) * pixpitch);
				head.Normalize((y - height / 2.0) * pixpitch);

				Math::Vector3d ray = target + wing + head - m_Scene.GetCamera();
				ray.Normalize();

		//			std::cout << "Ray " << ray << std::endl;

				colorf color;
				if (Cast(color, Ray(m_Scene.GetCamera(), ray)) == false) {
					// exited the world
					color = 0;
		//				float angle = Dot(Vector3d(0.0, 0.0, 1.0), ray);
		//				std::cout << "ray " << ray << " dot " << angle << std::endl;
		//				color = angle;
				}
				unit.SetResult(x, y, color);
			}
		}

		m_Master.ReturnWorkUnit(unit);
	}

	std::cout << "end trace" << std::endl;
}

