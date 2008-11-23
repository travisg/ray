#ifndef __RAY_H
#define __RAY_H

#include <math/Vector3.h>

struct Ray {
	Ray() {}
	Ray(const Math::Vector3 &_origin, const Math::Vector3 &_dir) : origin(_origin), dir(_dir) {}

	Math::Vector3 origin;
	Math::Vector3 dir;
};

#endif
