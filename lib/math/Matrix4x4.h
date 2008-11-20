#ifndef __MATRIX4X4_H
#define __MATRIX4X4_H

//#include <shared/defines.h>
#include <iostream>

namespace Math {

class Vector3;
class Vector4;
	
class Matrix4x4
{
public:
	Matrix4x4() {}
	Matrix4x4(const float[16]);

	/* constructors */
//	Matrix4x4(const Matrix4x4 &v);
//	Matrix4x4 &operator=(const Matrix4x4 &v);

	static const Matrix4x4 Identity;

	/* accessors */
	float operator[](int idx) const;

	/* set the matrix to various ops */
	Matrix4x4 &SetIdentity();
	Matrix4x4 &SetScaling(const Vector3 &scale);
	Matrix4x4 &SetTranslation(const Vector3 &xlate);
	Matrix4x4 &SetRotationX(float angle);
	Matrix4x4 &SetRotationY(float angle);
	Matrix4x4 &SetRotationZ(float angle);
	Matrix4x4 &SetProjectionPerspective(float fovy, float aspect, float nearz, float farz);
	Matrix4x4 &SetProjectionOrtho(float left, float right, float bottom, float top, float nearz, float farz);

	/* operators */
	Matrix4x4 operator*(const Matrix4x4 &v) const;
	Matrix4x4 &operator*=(const Matrix4x4 &v);
	Vector4 operator*(const Vector4 &v) const;

	Matrix4x4 Inverse() const;

	Vector3 Transform(const Vector3 &point) const;

	operator const float*() const { return val; }

private:
	/* data */
	float val[16];
};

/* debugging */
std::ostream &operator<<(std::ostream &os, const Matrix4x4 &v);

/* inline bits */
inline float Matrix4x4::operator[](int idx) const
{
	return val[idx];
}

}

#endif

