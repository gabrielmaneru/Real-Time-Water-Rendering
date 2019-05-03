#pragma once
#include "scene_object.h"

struct BaseShape : scene_object
{
	virtual ~BaseShape() = default;
	virtual float get_distance(vec3) = 0;
};
struct Plane : BaseShape
{
	Plane(float height)
		: m_height(height) {}
	float get_distance(vec3 p)override;

	float m_height;
};
struct Sphere : BaseShape
{
	Sphere(vec3 pos, float radius)
		: m_pos(pos), m_radius(radius) {}
	float get_distance(vec3 p)override;

	vec3 m_pos;
	float m_radius;
};
