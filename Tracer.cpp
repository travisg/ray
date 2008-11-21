
#include <cstdlib>
#include <iostream>

#include <Ray.h>
#include <Tracer.h>

using Math::Vector3;

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
	m_Camera = Math::Vector3(1.0f, 1.0f, -1.0f);
	m_Target = Math::Vector3(0.0f, 0.0f, 0.0f);
#endif
}

Tracer::~Tracer()
{
}

int Tracer::Cast(colorf &color, const Vector3 &ray, const Vector3 &cam, int depth)
{
	
	return -1;
}

void Tracer::Trace()
{
	int width = m_Surface.Width();
	int height = m_Surface.Height();

	std::cout << "Camera " << m_Camera << std::endl;
	std::cout << "Target " << m_Target << std::endl;

	Vector3 lin = m_Target - m_Camera;
	lin.Normalize();
	std::cout << "Lin " << lin << std::endl;

	int multisample = 1;
	float pixpitch = 2.0/width;
	float subpitch = pixpitch / ((float)multisample + 1.0);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			Vector3 wing(lin.gety(), -lin.getx(), 0);

//			std::cout << "wing " << wing << std::endl;

			Vector3 head = Cross(lin, wing);

//			std::cout << "head " << head << std::endl;

			wing.Normalize((x - width / 2.0) * pixpitch );
			head.Normalize((y - height / 2.0) * pixpitch );

			Math::Vector3 ray = m_Target + wing + head - m_Camera;

			ray.Normalize();

//			std::cout << "Ray " << ray << std::endl;

			colorf color;
			if (Cast(color, ray, m_Camera, 1) < 0) {
				// exited the world
//				color = 0;
				float angle = Dot(Vector3(0.0, 0.0, 1.0), ray);
//				std::cout << "ray " << ray << " dot " << angle << std::endl;
				color = angle;
			}
			m_Surface.SetXY(x, y, color);
		}
	}

	std::cout << "end trace" << std::endl;
}

