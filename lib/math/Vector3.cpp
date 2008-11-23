#include <iostream>
#include <math/Math.h>
#include <math/Vector3.h>

namespace Math {

std::ostream &operator<<(std::ostream &os, const Vector3 &v)
{
	os << "[ " << v.getx() << " " << v.gety() << " " << v.getz() << " ]";
	return os;
}

float Vector3::Length() const
{
	return Math<float>::sqrt(x*x + y*y + z*z);
}

void Vector3::Normalize()
{
	float lenSquared = LengthSquared();

	if (lenSquared == 0.0f) {
		x = y = z = 0;
	} else {
		float invsqrt = 1.0f / Math<float>::sqrt(lenSquared);

		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
	}
}

void Vector3::Normalize(float n)
{
	float lenSquared = LengthSquared();

	if (lenSquared == 0.0f) {
		x = y = z = 0;
	} else {
		float invsqrt = n / Math<float>::sqrt(lenSquared);

		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
	}
}

Vector3 Cross(const Vector3 &v1, const Vector3 &v2)
{
	return Vector3(
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x);
}

}

