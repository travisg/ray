#include <iostream>
#include <math/Math.h>
#include <math/Vector4.h>

namespace Math {

std::ostream &operator<<(std::ostream &os, const Vector4 &v)
{
	os << "[ " << v.getx() << " " << v.gety() << " " << v.getz() << " " << v.getw() << " ]";
	return os;
}

Vector4 Vector4::operator+(const Vector4 &v) const
{
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4 &Vector4::operator+=(const Vector4 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

Vector4 Vector4::operator-(const Vector4 &f) const
{
	return Vector4(x - f.x, y - f.y, z - f.z, w - f.w);
}


Vector4 &Vector4::operator-=(const Vector4 &f)
{
	x -= f.x;
	y -= f.y;
	z -= f.z;
	w -= f.w;
	return *this;
}

Vector4 Vector4::operator*(const Vector4 &f) const
{
	return Vector4(x * f.x, y * f.y, z * f.z, w * f.w);
}

Vector4 &Vector4::operator*=(const Vector4 &f)
{ 
	x *= f.x;
	y *= f.y;
	z *= f.z;
	w *= f.w;
	return *this;
}

Vector4 Vector4::operator*(float scalar) const
{
	return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4 operator*(float scalar, const Vector4 &v)
{
	return Vector4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

Vector4 &Vector4::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

Vector4 Vector4::operator/(float scalar) const
{
	return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4 &Vector4::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
	return *this;
}

float Vector4::Length() const
{
	return Math<float>::sqrt(x*x + y*y + z*z + w*w);
}

float Vector4::LengthSquared() const
{
	return (x*x + y*y + z*z + w*w);
}

float Vector4::Dot(const Vector4 &v) const
{
	return (x*v.x + y*v.y + z*v.z + w*v.w);	
}

float Dot(const Vector4 &v1, const Vector4 &v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);	
}

void Vector4::Normalize()
{
	float lenSquared = LengthSquared();

	if (lenSquared == 0.0f) {
		x = y = z = w = 0;
	} else {
		float invsqrt = 1.0f / Math<float>::sqrt(lenSquared);

		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
		w *= invsqrt;
	}
}

}

