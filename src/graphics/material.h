/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	material.h
Purpose: Material structure
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <string>
#include <glm/glm.h>
#include "shader_program.h"
#include "texture.h"

constexpr int default_material = -1;
struct Material
{
	void set_uniform(Shader_Program*)const;
	std::string m_name;

	vec3 m_albedo;
	Texture m_albedo_txt;

	vec3 m_metallic;
	Texture m_metallic_txt;

	float m_roughness;
	Texture m_roughness_txt;

	float m_ambient;
	Texture m_normal_txt;
};