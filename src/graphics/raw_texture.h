#pragma once
#include <glm/glm.h>
#include <utils/map2d.h>
struct raw_texture_rgb : public map2d<vec3>
{
	void load();
	unsigned int m_id{0};
};
struct raw_texture_single : public map2d<float>
{
	void load();
	unsigned int m_id{ 0 };
};