#include "scene_object.h"
void scene_object::draw(Shader_Program * shader)
{
	shader->set_uniform("M", m_transform.m_tr.get_model());
	m_model->draw(shader);
}
