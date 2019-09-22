#include "scene_object.h"
#include "graphics/renderer.h"
scene_object::scene_object(std::string mesh, transform3d tr)
	:renderable({ tr,renderer->get_model(mesh) }) {}

void scene_object::draw(Shader_Program * shader)
{
	shader->set_uniform("M", m_transform.m_tr.get_model());
	if(m_model != nullptr) m_model->draw(shader);
}
