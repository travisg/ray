#include <DefaultShader.h>
#include <Scene.h>
#include <Ray.h>

#include <iostream>

using Math::Vector3;

DefaultShader::DefaultShader()
{
}

DefaultShader::~DefaultShader()
{
}

colorf DefaultShader::Run(const ShaderArgs &args)
{
//	std::cout << "DefaultShader Run" << std::endl;

	// cast a ray at the sun, see if we're in a shadow
	Ray ray;
	ray.origin = args.pos;
	ray.dir = Vector3(0.0, 0.0, 1000.0) - ray.origin;
	ray.dir.Normalize();

	colorf color;
	if (args.scene->DoesIntersect(ray)) {
		color = 0;
	} else {
		float light = Dot(Vector3(0.0, 0.0, 1.0), args.normal);
	
		color = light;
	}

	return color;
}

