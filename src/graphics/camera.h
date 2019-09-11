#pragma once
#include <glm/glm.h>
constexpr struct {
	vec3 eye{ 0,0,10 };
	vec3 front{ 0,0,-1 };
	vec3 up{ 0,1,0 };
	vec4 viewrect{ -1,1,-1,1 };
	float yaw{ -90.0f };
	float pitch{ 0.0f };

}def_cam;

class camera
{
protected:
	camera(vec3 eye, vec3 front, vec3 up)
		:m_eye(eye), m_front(front), m_up(up) {}

public:
	virtual void update() = 0;

	vec3 m_eye;
	vec3 m_front;
	vec3 m_up;

	mat4 m_proj;
	mat4 m_view;
};