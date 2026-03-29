#pragma once
#include "Mat4.hpp"

constexpr float PI = 3.14159265358979323846f;

constexpr float toRadians(float degrees)
{
	return degrees * (PI / 180.0f);
}

constexpr float toDegrees(float radians)
{
	return radians * (180.0f / PI);
}

/// @brief Class containing the implementation of some methods from glm. Matrices are column-major.
class Math
{
	/// @brief Translates the given matrix m with the translation vector v.
	///	Instead of defining the transformation matrix of v and then multiplying it by m, it directly updates the last column.
	/// @param m Matrix to translate.
	/// @param v Translation vector.
	/// @return A new transformation matrix with the transformation applied.
	Mat4 translate(const Mat4 &m, const Vec3 &v)
	{
		Mat4 t = m;

		t.m[3] = m.m[0] * v.x + m.m[1] * v.y + m.m[2] * v.z + m.m[3];

		return t;
	}

	/// @brief Rotates the given matrix angle radians around the given axis.
	/// @param m Matrix to rotate.
	/// @param angle Angle to rotate.
	/// @param axis Axis to rotate around.
	/// @return A new transformation matrix with the rotation applied.
	Mat4 rotate(const Mat4 &m, float angle, const Vec3 &axis)
	{
		Vec3 a = axis.normalized();
		float x = a.x, y = a.y, z = a.z;

		float c = cos(angle);
		float s = sin(angle);
		float t = 1.0f - c;

		Mat4 r(1.0f);

		r.m[0] = Vec4(t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0);
		r.m[1] = Vec4(t * x * y - s * z, t * y * y + c, t * y * z + s * x, 0);
		r.m[2] = Vec4(t * x * z + s * y, t * y * z - s * x, t * z * z + c, 0);
		r.m[3] = Vec4(0, 0, 0, 1);

		Mat4 result;
		result.m[0] = m.m[0] * r.m[0].x + m.m[1] * r.m[0].y + m.m[2] * r.m[0].z;
		result.m[1] = m.m[0] * r.m[1].x + m.m[1] * r.m[1].y + m.m[2] * r.m[1].z;
		result.m[2] = m.m[0] * r.m[2].x + m.m[1] * r.m[2].y + m.m[2] * r.m[2].z;
		result.m[3] = m.m[3];

		return result;
	}
};