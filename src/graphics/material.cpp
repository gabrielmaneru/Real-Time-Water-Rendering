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
