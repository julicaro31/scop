#pragma once
#include "Mat4.hpp"

/// @brief Class containing the implementation of some methods from glm. Matrices are column-major.
class Math
{
	/// @brief Translates the give matrix m with the translation vector v.
	///	Instead of defining the transformation matrix of v and then multiplying it by m, it directly updates the last column.
	/// @param m Matrix to tranlate
	/// @param v Tranlation vector
	/// @return A new transformation matrix with the new transformation applied.
	Mat4 translate(const Mat4 &m, const Vec3 &v)
	{
		Mat4 t = m;

		t.m[3][0] = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0];
		t.m[3][1] = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1];
		t.m[3][2] = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2];
		t.m[3][3] = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3];

		return t;
	}
};