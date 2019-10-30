/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	camera.h
Purpose: Camera Base
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <glm/glm.h>
class Shader_Program;
constexpr struct {
	vec3 eye{ 0,0,10 };
	vec3 front{ 0,0,-1 };
	vec3 up{ 0,1,0 };
	vec4 viewrect{ -1,1,-1,1 };
	float yaw{ 180.0f };
	float pitch{ 0.0f };

}def_cam;

class camera
{
protected:
	camera(vec3 eye, vec3 front, vec3 up)
		:m_eye(eye), m_front(front), m_up(up) {}

public:
	virtual void update() = 0;
	void set_uniforms(Shader_Program*);
	void set_prev_uniforms(Shader_Program*);
	void save_prev() { m_view_prev = m_view; }

	vec3 m_eye;
	vec3 m_front;
	vec3 m_up;

	mat4 m_proj;
	mat4 m_view;
	mat4 m_view_prev;
};