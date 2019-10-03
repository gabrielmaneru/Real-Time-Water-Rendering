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
scene_object::scene_object(std::string mesh, transform3d tr)
	:renderable(tr,renderer->get_model(mesh)) {}

void scene_object::draw(Shader_Program * shader)
{
	shader->set_uniform("M", m_transform.m_tr.get_model());
	shader->set_uniform("selection_color", renderer->compute_selection_color());
	if(m_model != nullptr)
		m_model->draw(shader);
}
