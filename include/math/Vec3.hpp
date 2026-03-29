#pragma once
#include <cmath>

struct Vec3
{
	float x, y, z;

	Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3 operator+(const Vec3 &v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	Vec3 operator-(const Vec3 &v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3 operator*(float s) const
	{
		return Vec3(x * s, y * s, z * s);
	}

	float dot(const Vec3 &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3 cross(const Vec3 &v) const
	{
		return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	float length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	void normalize()
	{
		float len = length();
		if (len > 0.0f)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	Vec3 Vec3::normalized() const
	{
		float len = std::sqrt(x * x + y * y + z * z);

		if (len > 0.0f)
		{
			return Vec3(x / len, y / len, z / len);
		}

		return Vec3(0, 0, 0);
	}
};