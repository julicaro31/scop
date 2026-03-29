#pragma once
#include "Vec4.hpp"

/// @brief Implementation of a 4x4 column-major matrix.
struct Mat4
{
	Vec4 m[4]; // column-major: m[col]

	Mat4(float n = 0.0f)
	{
		m[0] = Vec4(n, 0, 0, 0);
		m[1] = Vec4(0, n, 0, 0);
		m[2] = Vec4(0, 0, n, 0);
		m[3] = Vec4(0, 0, 0, n);
	}

	Vec4 operator*(const Vec4 &v) const
	{
		return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
	}

	Mat4 operator*(const Mat4 &a) const
	{
		Mat4 result;

		result.m[0] = (*this) * a.m[0];
		result.m[1] = (*this) * a.m[1];
		result.m[2] = (*this) * a.m[2];
		result.m[3] = (*this) * a.m[3];

		return result;
	}
};