#include <DefaultShader.h>
#include <Scene.h>
#include <Ray.h>
#include <SimpleLight.h>

#include <iostream>

using Math::Vector3f;

colorf ambient = 0.10f;

DefaultShader::DefaultShader()
{
}

DefaultShader::~DefaultShader()
{
}

colorf DefaultShader::Run(const ShaderArgs &args)
{
//	std::cout << "DefaultShader Run" << std::endl;

	colorf color = 0;
	std::vector<SimpleLight *> lightList = args.scene->GetLightList();

	for (std::vector<SimpleLight *>::const_iterator i = lightList.begin(); i != lightList.end(); i++) {
		const SimpleLight *l = *i;

		// cast a ray at each light, see if we're in a shadow
		Ray ray;
		ray.origin = args.pos;
		ray.dir = l->GetPos() - ray.origin;
		ray.dir.Normalize();
	
		if (!args.scene->DoesIntersect(ray)) {
			Vector3f suntosurface = l->GetPos() - args.pos;
			suntosurface.Normalize();
		
			float light = Dot(suntosurface, args.normal);

			// calculate falloff

			color += l->GetColor() * light;
		}
	}

	return color;
}

