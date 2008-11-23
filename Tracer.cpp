
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <vector>

#include <Scene.h>
#include <Ray.h>
#include <Tracer.h>
#include <Sphere.h>

#include <Shader.h>

using Math::Vector3;

colorf ambient = 0.10f;

Tracer::Tracer(RenderSurface &surface, Scene &scene)
:	m_Surface(surface),
	m_Scene(scene)
{
#if 0
	m_Camera = Math::Vector3(
		(double)(rand()%900) + 50.0,
		(double)(rand()%900) + 50.0,
		100.0);

	m_Target = Math::Vector3(
		(double)(rand()%900) + 50.0,
		(double)(rand()%900) + 50.0,
    	m_Camera.getz() - (double)(rand()%400));
#else
	m_Camera = Math::Vector3(100.0f, 100.0f, 100.0f);
	m_Target = Math::Vector3(0.0f, 0.0f, 0.0f);
#endif
}

Tracer::~Tracer()
{
}

bool Tracer::Cast(colorf &color, const Ray &ray)
{
	// tell the scene graph to find an object to intersect
	const Drawable *d = m_Scene.Intersect(ray);

	// render the closest one
	if (d) {
		Vector3 pos;
		Vector3 normal;
		d->Intersect(ray, pos, normal);

		boost::shared_ptr<Shader> s = d->GetShader();
		if (!s) {
			color = 0;
		} else {
			ShaderArgs args;

			args.scene = &m_Scene;
			args.d = d;
			args.pos = pos;
			args.normal = normal;
			color = s->Run(args);
		}

		color += ambient;

		return true;
	}

	return false;
}

void Tracer::Trace()
{
	int width = m_Surface.Width();
	int height = m_Surface.Height();

	std::cout << "Camera " << m_Camera << std::endl;
	std::cout << "Target " << m_Target << std::endl;

	m_Target -= m_Camera;
	m_Target.Normalize();
	m_Target += m_Camera;

	Vector3 lin = m_Target - m_Camera;

	std::cout << "Lin " << lin << std::endl;

//	int multisample = 1;
	float pixpitch = 2.0/width;
//	float subpitch = pixpitch / ((float)multisample + 1.0);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Vector3 wing(lin.gety(), -lin.getx(), 0);

//			std::cout << "wing " << wing << std::endl;

			Vector3 head = Cross(lin, wing);

//			std::cout << "head " << head << std::endl;

			wing.Normalize((x - width / 2.0) * pixpitch);
			head.Normalize((y - height / 2.0) * pixpitch);

			Math::Vector3 ray = m_Target + wing + head - m_Camera;

			ray.Normalize();

//			std::cout << "Ray " << ray << std::endl;

			colorf color;
			if (Cast(color, Ray(m_Camera, ray)) == false) {
				// exited the world
				color = 0;
//				float angle = Dot(Vector3(0.0, 0.0, 1.0), ray);
//				std::cout << "ray " << ray << " dot " << angle << std::endl;
//				color = angle;
			}
			m_Surface.SetXY(x, y, color);
		}
	}

	std::cout << "end trace" << std::endl;
}

