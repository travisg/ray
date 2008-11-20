#include <math.h>
#include <math/Math.h>

namespace Math {

/* some common math routines */
template <>
float Math<float>::sqrt(float root)
{
	return ::sqrtf(root);
}

template <>
float Math<float>::sin(float root)
{
	return ::sinf(root);
}

template <>
float Math<float>::cos(float root)
{
	return ::cosf(root);
}

template <>
float Math<float>::tan(float root)
{
	return ::tanf(root);
}

}
