/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	material.cpp
Purpose: Material structure
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "material.h"
#include <gl/gl3w.h>
Material::Material(std::string name, vec3 albedo_v, std::string albedo_path, vec3 metalli_v, std::string metallic_path, float roughness_v, std::string roughness_path, float ambient_v, std::string normal_path)
	:m_name(name)
{
	if (!albedo_path.empty())
	{
		size_t start = albedo_path.find_last_of('/') + 1;
		if (start > albedo_path.size()) start = 0;
		if (start == 0)
		{
			start = albedo_path.find_last_of('\\') + 1;
			if (start > albedo_path.size()) start = 0;
		}
		albedo_path = "./data/textures/" + albedo_path.substr(start);

		m_albedo_txt.loadFromFile(albedo_path.c_str());
		m_albedo_txt.m_path = albedo_path.c_str();
	}
	else
		m_albedo = albedo_v;

	if (!metallic_path.empty())
	{
		size_t start = metallic_path.find_last_of('/') + 1;
		if (start > metallic_path.size()) start = 0;
		if (start == 0)
		{
			start = metallic_path.find_last_of('\\') + 1;
			if (start > metallic_path.size()) start = 0;
		}
		metallic_path = "./data/textures/" + metallic_path.substr(start);

		m_metallic_txt.loadFromFile(metallic_path.c_str());
		m_metallic_txt.m_path = metallic_path.c_str();
	}
	else
		m_metallic = metalli_v;

	if (!roughness_path.empty())
	{
		size_t start = roughness_path.find_last_of('/') + 1;
		if (start > roughness_path.size()) start = 0;
		if (start == 0)
		{
			start = roughness_path.find_last_of('\\') + 1;
			if (start > roughness_path.size()) start = 0;
		}
		roughness_path = "./data/textures/" + roughness_path.substr(start);

		m_roughness_txt.loadFromFile(roughness_path.c_str());
		m_roughness_txt.m_path = roughness_path.c_str();
	}
	else
		m_roughness = roughness_v;

	m_ambient = ambient_v;

	if (!normal_path.empty())
	{
		size_t start = normal_path.find_last_of('/') + 1;
		if (start > normal_path.size()) start = 0;
		if (start == 0)
		{
			start = normal_path.find_last_of('\\') + 1;
			if (start > normal_path.size()) start = 0;
		}
		normal_path = "./data/textures/" + normal_path.substr(start);

		m_normal_txt.loadFromFile(normal_path.c_str());
		m_normal_txt.m_path = normal_path.c_str();
	}
}
void Material::set_uniform(Shader_Program * shader)const
{

	shader->set_uniform("kalbedo", m_albedo);
	if (m_albedo_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		shader->set_uniform("albedo_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_albedo_txt.m_id);
	}
	else
		shader->set_uniform("albedo_txt_active", false);

	shader->set_uniform("kmetallic", m_metallic);
	if (m_metallic_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		shader->set_uniform("metallic_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_metallic_txt.m_id);
	}
	else
		shader->set_uniform("metallic_txt_active", false);

	shader->set_uniform("kroughness", m_roughness);
	if (m_roughness_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		shader->set_uniform("roughness_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_roughness_txt.m_id);
	}
	else
		shader->set_uniform("roughness_txt_active", false);

	shader->set_uniform("kambient", m_ambient);
	if (m_normal_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE3);
		shader->set_uniform("normal_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_normal_txt.m_id);
	}
	else
		shader->set_uniform("normal_txt_active", false);
}
