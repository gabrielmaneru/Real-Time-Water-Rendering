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
	shader->set_uniform("ka", m_ambient);
	shader->set_uniform("kd", m_diffuse);
	shader->set_uniform("ks", m_specular);

	shader->set_uniform("ns", m_shininess);
	
	if (m_diffuse_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		shader->set_uniform_sampler(0);
		shader->set_uniform("diff_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_diffuse_txt.m_id);
	}
	else
		shader->set_uniform("diff_txt_active", false);

	if (m_specular_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		shader->set_uniform("spec_txt_active", true);
		shader->set_uniform_sampler(1);
		glBindTexture(GL_TEXTURE_2D, m_specular_txt.m_id);
	}
	else
		shader->set_uniform("spec_txt_active", false);
	
	if (m_normal_txt.m_id > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		shader->set_uniform_sampler(2);
		shader->set_uniform("norm_txt_active", true);
		glBindTexture(GL_TEXTURE_2D, m_normal_txt.m_id);
	}
	else
		shader->set_uniform("norm_txt_active", false);
}
