#ifndef __SHADER_H
#define __SHADER_H

#include <Color.h>
#include <math/Vector3.h>

class Drawable;
class Scene;

struct ShaderArgs {
	Scene *scene;
	const Drawable *d;
	Math::Vector3 pos;
	Math::Vector3 normal;
};

class Shader {
public:
	Shader() {}
	virtual ~Shader() {}

	virtual colorf Run(const ShaderArgs &args) = 0;
};

#endif

