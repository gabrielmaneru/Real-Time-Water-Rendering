/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene_object.cpp
Purpose: Object base class
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/
#include "scene_object.h"
#include "graphics/renderer.h"

scene_object::scene_object(std::string mesh, transform3d tr, animator * anim)
	: renderable(tr, renderer->get_model(mesh)), m_animator(anim) {}

void scene_object::draw(Shader_Program * shader)
{
	m_transform.m_tr.save_prev();
	shader->set_uniform("M", m_transform.m_tr.get_model());
	if(m_model != nullptr)
		m_model->draw(shader, m_animator);
}
