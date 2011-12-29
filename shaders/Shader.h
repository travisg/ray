#ifndef __SHADER_H
#define __SHADER_H

#include <Color.h>
#include <libvec/Vector3.h>
#include <boost/shared_ptr.hpp>

class Drawable;
class Scene;
class Tracer;
struct Ray;

struct ShaderArgs {
	Scene *scene;
	Tracer *tracer;
	const Drawable *d;
	const Ray *ray;
	Libvec::Vector3d pos;
	Libvec::Vector3d normal;
};

class Shader {
public:
	Shader() {}
	virtual ~Shader() {}

	virtual colorf Run(const ShaderArgs &args) = 0;
};

typedef boost::shared_ptr<Shader> ShaderPtr;

#endif

