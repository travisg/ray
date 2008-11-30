#ifndef __RAY_H
#define __RAY_H

#include <libmath/Vector3.h>

struct Ray {
	Ray() {}
	Ray(const Math::Vector3f &_origin, const Math::Vector3f &_dir) : origin(_origin), dir(_dir) {}

	Math::Vector3f origin;
	Math::Vector3f dir;
};

#endif
