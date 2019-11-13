/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	light.h
Purpose: Light base class
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "scene_object.h"
struct light_data
{
	static vec3 m_ambient;
	static float m_att_max;
	vec3 m_diffuse{1.0f};
	vec3 m_att_factor{.0f, .05f, .02f};
	void drawGUI();
};

class point_light : public scene_object
{
	void recompute_scale();
public:
	point_light(transform3d tr = {}, light_data ld = {});
	virtual ~point_light() = default;
	void draw(Shader_Program*);

	light_data m_ldata;
};

class dir_light : public scene_object
{
public:
	dir_light(vec3 dir, transform3d tr = {}, light_data ld = {});
	virtual ~dir_light() = default;
	void draw(Shader_Program*);

	light_data m_ldata;
	vec3 m_direction;
};