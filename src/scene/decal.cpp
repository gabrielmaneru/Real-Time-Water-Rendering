#include "decal.h"

decal::decal(std::string diffuse, std::string normal, transform3d tr)
	:scene_object("cube", tr,nullptr,nullptr)
{
	m_diffuse.m_path = Texture::filter_name(diffuse);
	m_diffuse.loadFromFile(m_diffuse.m_path.c_str(), false);

	m_normal.m_path = Texture::filter_name(normal);
	m_normal.loadFromFile(m_normal.m_path.c_str(), false);
}

void decal::draw(Shader_Program * shader)
{
	glActiveTexture(GL_TEXTURE0);
	shader->set_uniform("albedo_txt_active", true);
	glBindTexture(GL_TEXTURE_2D, m_diffuse.m_id);

	shader->set_uniform("kmetallic", vec3(0.0f));
	shader->set_uniform("metallic_txt_active", false);

	shader->set_uniform("kroughness", 0.0f);
	shader->set_uniform("roughness_txt_active", false);

	shader->set_uniform("kambient", 1.0f);
	glActiveTexture(GL_TEXTURE3);
	shader->set_uniform("normal_txt_active", true);
	glBindTexture(GL_TEXTURE_2D, m_normal.m_id);

	if (m_model)
	{
		shader->set_uniform("M", m_transform.get_model());
		m_model->draw(shader, nullptr, false);
	}
}
