#ifndef __VECTOR3_H
#define __VECTOR3_H

//#include <shared/defines.h>
#include <ostream>

namespace Math {

class Vector3
{
public:
	inline Vector3() {}
	inline ~Vector3() {}

	Vector3(float x1, float y1, float z1);

	/* constructors */
	Vector3(const Vector3 &v);
	Vector3 &operator=(const Vector3 &v);

	/* accessors */
	float getx() const;
	float gety() const;
	float getz() const;

	/* operators */
	float* operator*() const;
	Vector3 operator+(const Vector3 &v) const;
	Vector3 &operator+=(const Vector3 &v);
	Vector3 operator-(const Vector3 &v) const;
	Vector3 operator-() const;
	Vector3 &operator-=(const Vector3 &v);
	Vector3 operator*(const Vector3 &v) const;
	Vector3 &operator*=(const Vector3 &v);

	/* scalar ops */
	Vector3 operator*(float scalar) const;
	friend Vector3 operator*(float scalar, const Vector3 &v);
	Vector3 &operator*=(float scalar);
	Vector3 operator/(float scalar) const;
	Vector3 &operator/=(float scalar);

	/* common vector stuff */
	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector3 &v) const;
	friend float Dot(const Vector3 &v1, const Vector3 &v2);
	void Normalize();
	void Normalize(float n);

	friend Vector3 Cross(const Vector3 &v1, const Vector3 &v2);

	operator const float*() const { return &x; }

private:
	/* data */
	float x, y, z, _w;
};

/* debugging */
std::ostream &operator<<(std::ostream &os, const Vector3 &v);

/* inline constructors */
inline Vector3::Vector3(float x1, float y1, float z1)
{
	x = x1;
	y = y1;
	z = z1;
}

inline Vector3::Vector3(const Vector3 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

inline Vector3 &Vector3::operator=(const Vector3 &v)
{
	if (this == &v)
		return *this;

	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

inline float Vector3::getx() const { return x; }
inline float Vector3::gety() const { return y; }
inline float Vector3::getz() const { return z; }

inline Vector3 Vector3::operator+(const Vector3 &v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

inline Vector3 &Vector3::operator+=(const Vector3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline Vector3 Vector3::operator-(const Vector3 &f) const
{
	return Vector3(x - f.x, y - f.y, z - f.z);
}

inline Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

inline Vector3 &Vector3::operator-=(const Vector3 &f)
{
	x -= f.x;
	y -= f.y;
	z -= f.z;
	return *this;
}

inline Vector3 Vector3::operator*(const Vector3 &f) const
{
	return Vector3(x * f.x, y * f.y, z * f.z);
}

inline Vector3 &Vector3::operator*=(const Vector3 &f)
{ 
	x *= f.x;
	y *= f.y;
	z *= f.z;
	return *this;
}

inline Vector3 Vector3::operator*(float scalar) const
{
	return Vector3(x * scalar, y * scalar, z * scalar);
}

inline Vector3 operator*(float scalar, const Vector3 &v)
{
	return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}

inline Vector3 &Vector3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

inline Vector3 Vector3::operator/(float scalar) const
{
	return Vector3(x / scalar, y / scalar, z / scalar);
}

inline Vector3 &Vector3::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

inline float Vector3::LengthSquared() const
{
	return (x*x + y*y + z*z);
}

inline float Vector3::Dot(const Vector3 &v) const
{
	return (x*v.x + y*v.y + z*v.z);	
}

inline float Dot(const Vector3 &v1, const Vector3 &v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);	
}

}

#endif

