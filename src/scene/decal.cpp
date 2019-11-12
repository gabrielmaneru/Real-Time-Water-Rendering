#include "decal.h"
#include <graphics/renderer.h>

decal::decal(std::string diffuse, std::string normal, transform3d tr)
	:scene_object("cube", tr,nullptr,nullptr)
{
	m_diffuse.m_path = Texture::filter_name(diffuse);
	m_diffuse.loadFromFile(m_diffuse.m_path.c_str());

	m_normal.m_path = Texture::filter_name(normal);
	m_normal.loadFromFile(m_normal.m_path.c_str());
}

void decal::draw(Shader_Program * shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_diffuse.m_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normal.m_id);

	if (m_model)
	{
		shader->set_uniform("M", m_transform.get_model());
		m_model->draw(shader, nullptr, false);
	}
}
