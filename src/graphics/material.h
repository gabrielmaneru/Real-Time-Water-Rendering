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
	Material() = default;
	Material(std::string name,
		vec3 albedo_v, std::string albedo_path,
		vec3 metalli_v, std::string metallic_path,
		float roughness_v, std::string roughness_path,
		float ambient_v, std::string normal_path);
	Material(const Material&) = default;
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