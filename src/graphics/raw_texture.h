/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	raw_texture.h
Purpose: Basic texture handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <glm/glm.h>
#include <utils/map2d.h>
struct raw_texture_rgb : public map2d<vec3>
{
	void load();
	raw_texture_rgb& operator=(const map2d<vec3>& map);

	unsigned int m_id{0};
};
struct raw_texture_single : public map2d<float>
{
	void load();
	raw_texture_single& operator=(const map2d<float>& map);

	unsigned int m_id{ 0 };
};