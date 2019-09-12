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
	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;
	float m_shininess;
	Texture m_diffuse_txt;
	Texture m_specular_txt;
	Texture m_normal_txt;
};