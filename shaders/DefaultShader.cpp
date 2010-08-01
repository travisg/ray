#include <iostream>

#include <Scene.h>
#include <Ray.h>
#include <Tracer.h>
#include <lights/SimpleLight.h>
#include "DefaultShader.h"

using Math::Vector3d;

DefaultShader::DefaultShader()
	: m_Shinyness(0.0f),
	  m_DiffuseColor(1.0f, 1.0f, 1.0f)
{
}

DefaultShader::~DefaultShader()
{
}

colorf DefaultShader::Run(const ShaderArgs &args)
{
//	std::cout << "DefaultShader Run" << std::endl;

	colorf color = 0.0;
	std::vector<SimpleLight *> lightList = args.scene->GetLightList();

	for (std::vector<SimpleLight *>::const_iterator i = lightList.begin(); i != lightList.end(); i++) {
		const SimpleLight *l = *i;

		// cast a ray at each light, see if we're in a shadow
		Ray ray;
		ray.origin = args.pos;
		ray.dir = l->GetPos() - ray.origin;
		ray.dir.Normalize();
	
		if (!args.scene->DoesIntersect(ray, (ray.origin - l->GetPos()).Length())) {
			Vector3d suntosurface = l->GetPos() - args.pos;
			suntosurface.Normalize();
		
			float light = Dot(suntosurface, args.normal);

			// calculate falloff

			color += (l->GetColor() * light) * m_DiffuseColor;
		}
	}

	// see how much of it is a pure reflection
	if (m_Shinyness > 0.0f) {
		Ray ray;
		ray.origin = args.pos;

		// calculate the reflection ray
		float d = Dot(args.ray->dir, args.normal);

		Vector3d reflect(
			args.ray->dir.getx() - 2.0 * d * args.normal.getx(),
			args.ray->dir.gety() - 2.0 * d * args.normal.gety(),
			args.ray->dir.getz() - 2.0 * d * args.normal.getz());

		ray.dir = reflect;

		// recursively trace to see
		colorf reflectcolor;
		args.tracer->Cast(reflectcolor, ray);

		color *= (1.0f - m_Shinyness);
		color += reflectcolor * m_Shinyness;
	}

	return color;
}

