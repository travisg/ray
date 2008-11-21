#ifndef __MATH_MATH_H
#define __MATH_MATH_H

//#include <shared/defines.h>
#include <math/Vector4.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>

namespace Math {

template<class T>
class Math {
public:
	static T sqrt(T root);
	static T sin(T val);
	static T cos(T val);
	static T tan(T val);

private:
	Math<T>();
};

#define F_PI 3.14159f

inline float DegreeToRadians(float deg)
{
	return (deg * F_PI) / 180.0f;
}

}

#endif

