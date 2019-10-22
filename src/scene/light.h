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

class light : public scene_object
{
	void recompute_scale();
public:
	light(transform3d tr = {}, light_data ld={});
	virtual ~light() = default;
	void draw(Shader_Program*);

	float time{0.0f};
	light_data m_ldata;
};