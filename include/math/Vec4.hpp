#pragma once
#include "Vec3.hpp"

struct Vec4
{
	float x, y, z, w;

	Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
	Vec4(Vec3 &vec3, float w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}
};