/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	camera.cpp
Purpose: Camera Base
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#include "camera.h"
#include <graphics/shader_program.h>

void camera::set_uniforms(Shader_Program * shader)
{
	shader->set_uniform("P", m_proj);
	shader->set_uniform("V", m_view);
	shader->set_uniform("M", mat4(1.0f));
}

void camera::set_prev_uniforms(Shader_Program * shader)
{
	shader->set_uniform("V_prev", m_view_prev);
	shader->set_uniform("M_prev", mat4(1.0f));
}
