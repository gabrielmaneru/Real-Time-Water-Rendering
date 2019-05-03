#pragma once
#include <glm/glm.h>
struct ortho_camera
{
	void update();

	vec4 view_rect;

	vec3 eye;
	vec3 target;
	vec3 up{ 0.0f, 1.0f, 0.0f };

	mat4 proj{};
	mat4 view{};
};