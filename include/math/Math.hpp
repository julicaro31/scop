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
	static Mat4 translate(const Mat4 &m, const Vec3 &v)
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
	static Mat4 rotate(const Mat4 &m, float angle, const Vec3 &axis)
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

	/// @brief Scales the given matrix by the given scaling vector.
	/// @param m Matrix to scale.
	/// @param v Scaling vector.
	/// @return A new transformation matrix with the scaling applied.
	static Mat4 scale(const Mat4 &m, const Vec3 &v)
	{
		Mat4 result;

		result.m[0] = m.m[0] * v.x;
		result.m[1] = m.m[1] * v.y;
		result.m[2] = m.m[2] * v.z;
		result.m[3] = m.m[3];

		return result;
	}

	/// @brief Builds a perspective projection matrix that maps the 3D viewing frustum to OpenGL's normalized clip space [-1, 1].
	///        Creates the depth illusion where far objects appear smaller, by setting up the w component for perspective division.
	/// @param fov Vertical field of view angle in radians.
	/// @param aspect Width-to-height ratio of the viewport (e.g. 800/600).
	/// @param near Distance to the near clipping plane (must be > 0).
	/// @param far Distance to the far clipping plane (must be > near).
	/// @return A 4x4 perspective projection matrix.
	static Mat4 perspective(float fov, float aspect, float near, float far)
	{
		float tanHalf = tan(fov / 2.0f);
		Mat4 result(0.0f);

		result.m[0].x = 1.0f / (aspect * tanHalf);
		result.m[1].y = 1.0f / tanHalf;
		result.m[2].z = -(far + near) / (far - near);
		result.m[2].w = -1.0f;
		result.m[3].z = -(2.0f * far * near) / (far - near);

		return result;
	}

	/// @brief Builds a view matrix that positions and orients the camera in the scene.
	///        Constructs an orthonormal basis (right, up, forward) from the camera's position and target,
	///        then combines a rotation to align the axes with a translation to move the world opposite to the camera's position.
	/// @param eye Position of the camera in world space.
	/// @param center The point the camera is looking at.
	/// @param up The world's up direction (typically 0, 1, 0).
	/// @return A 4x4 view matrix.
	static Mat4 lookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
	{
		Vec3 f = (center - eye).normalized();
		Vec3 r = f.cross(up).normalized();
		Vec3 u = r.cross(f);

		Mat4 result(1.0f);

		result.m[0].x = r.x;
		result.m[1].x = r.y;
		result.m[2].x = r.z;

		result.m[0].y = u.x;
		result.m[1].y = u.y;
		result.m[2].y = u.z;

		result.m[0].z = -f.x;
		result.m[1].z = -f.y;
		result.m[2].z = -f.z;

		result.m[3].x = -r.dot(eye);
		result.m[3].y = -u.dot(eye);
		result.m[3].z = f.dot(eye);

		return result;
	}
};