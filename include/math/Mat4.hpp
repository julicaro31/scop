#pragma once
#include "Vec4.hpp"

struct Mat4
{
	float m[4][4];

	Mat4(float n = 0.0f)
	{
		for (int col = 0; col < 4; ++col)
			for (int row = 0; row < 4; ++row)
				m[col][row] = (col == row) ? n : 0.0f;
	}

	Mat4 operator*(const Mat4 &a) const
	{
		Mat4 result;

		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				result.m[col][row] =
					m[0][row] * a.m[col][0] +
					m[1][row] * a.m[col][1] +
					m[2][row] * a.m[col][2] +
					m[3][row] * a.m[col][3];
			}
		}

		return result;
	}

	Vec4 operator*(const Vec4 &v) const
	{
		float x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
		float y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
		float z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
		float w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;

		return Vec4(x, y, z, w);
	}
};