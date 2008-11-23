
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <vector>

#include <Ray.h>
#include <Tracer.h>
#include <Sphere.h>

using Math::Vector3;

std::vector<Sphere *> slist;

colorf ambient = 0.10f;

Tracer::Tracer(RenderSurface &surface)
:	m_Surface(surface)
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

bool Tracer::Cast(colorf &color, const Ray &ray, bool collidetest)
{

	bool hit = false;
	Vector3 closestPos;
	Vector3 closestNormal;
	Sphere *closestSphere;

	// iterate over the list of objects, testing collision and tracking the closest one
	for (std::vector<Sphere *>::const_iterator i = slist.begin(); i != slist.end(); i++) {
		Sphere *s = *i;

		if (collidetest) {
			if (s->Intersect(ray)) {
				// if we're just trying to test a collision, bail with true here
				return true;
			}
		} else {
			Vector3 pos;
			Vector3 normal;
			if (s->Intersect(ray, pos, normal)) {
//				std::cout << "collision ray " << ray << " pos " << pos << std::endl;
				if (!hit || (pos - ray.origin).LengthSquared() < (closestPos - ray.origin).LengthSquared()) {
					closestPos = pos;
					closestNormal = normal;
					closestSphere = s;
				}
				hit = true;
			}
		}
	}

	// render the closest one
	if (hit) {
		// cast a ray at the sun, see if we're in a shadow
		Ray ray;
		ray.origin = closestPos;
		ray.dir = Vector3(0.0, 0.0, 1000.0) - ray.origin;
		ray.dir.Normalize();

		colorf c;
		if (Cast(c, ray, true)) {
			color = 0;
		} else {
			float light = Dot(Vector3(0.0, 0.0, 1.0), closestNormal);

			color = light;
		}

		color += ambient;
		
		return true;
	}

	return false;
}

void Tracer::Trace()
{
	int i;

#if 1
	for (i=0; i < 100; i++) {
		slist.push_back(new Sphere(Vector3((rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f, (rand()%1000 - 500) / 10.0f), (rand()%200) / 10.0f));
	}
#endif

	slist.push_back(new Sphere(Vector3(0.0, 0.0, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.0, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.5, 0.0), 0.25));
	slist.push_back(new Sphere(Vector3(0.5, 0.6, 0.5), 0.10));

	int width = m_Surface.Width();
	int height = m_Surface.Height();

	std::cout << "Camera " << m_Camera << std::endl;
	std::cout << "Target " << m_Target << std::endl;

	m_Target -= m_Camera;
	m_Target.Normalize();
	m_Target += m_Camera;

	Vector3 lin = m_Target - m_Camera;

	std::cout << "Lin " << lin << std::endl;

	int multisample = 1;
	float pixpitch = 2.0/width;
	float subpitch = pixpitch / ((float)multisample + 1.0);

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
			if (Cast(color, Ray(m_Camera, ray), false) == false) {
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

