#pragma once
#include "Mat4.hpp"

class Math
{

	// Mat4 translate(const Mat4 &m, const Vec3 &v)
	// {
	// 	Mat4 t = {
	// 		std::array<float, 4>{1, 0, 0, v[0]},
	// 		std::array<float, 4>{0, 1, 0, v[1]},
	// 		std::array<float, 4>{0, 0, 1, v[2]},
	// 		std::array<float, 4>{0, 0, 0, 1}};

	// 	return multiply(m, t);
	// }

	Mat4 translate(const Mat4 &m, const Vec3 &v)
	{
		Mat4 result = m;

		result.m[0][3] = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3];
		result.m[1][3] = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3];
		result.m[2][3] = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3];
		result.m[3][3] = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3];

		return result;
	}
};