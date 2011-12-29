#ifndef __RAY_H
#define __RAY_H

#include <libvec/Vector3.h>

struct Ray {
	Ray() {}
	Ray(const Libvec::Vector3d &_origin, const Libvec::Vector3d &_dir) : origin(_origin), dir(_dir) {}

	Libvec::Vector3d origin;
	Libvec::Vector3d dir;
};

#endif
