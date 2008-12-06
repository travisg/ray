#ifndef __RAY_H
#define __RAY_H

#include <libmath/Vector3.h>

struct Ray {
	Ray() {}
	Ray(const Math::Vector3d &_origin, const Math::Vector3d &_dir) : origin(_origin), dir(_dir) {}

	Math::Vector3d origin;
	Math::Vector3d dir;
};

#endif
