#pragma once
#include <glm/glm.h>
#include <vector>
struct raw_texture
{
	void setup(size_t width, size_t height);
	void clear(vec3 clear_color);
	void load();
	vec3 get(size_t x, size_t y)const;
	void set(size_t x, size_t y, vec3 value);

	unsigned int m_id{0};
	size_t m_width;
	size_t m_height;
	std::vector<vec3> m_pixels;
};