#include "shape.h"

float Plane::get_distance(vec3 p)
{
	return glm::abs(p.y - m_height);
}

float Sphere::get_distance(vec3 p)
{
	return glm::length(p - m_pos) - m_radius;
}
