#include "ik_bone.h"
#include <graphics/renderer.h>
#include <functional>

ik_chain::ik_chain(transform3d tr, size_t start_count)
	: scene_object("joint", tr, nullptr, nullptr)
{
	ik_bone * last{nullptr};
	for (size_t i = 0; i < start_count; i++)
	{

		ik_bone* b = new ik_bone(last);
		m_bones.push_back(b);
		if (last == nullptr)
			m_root = b;
		else
			last->m_children.push_back(b);
		last = b;
	}
}

void ik_chain::draw(Shader_Program * shader)
{
	for(auto& b : m_bones)
	{
		transform3d tr;
		tr.set_pos(b->get_relative_pos());
		tr.set_scl(vec3(b->m_length, 1, 1));
		tr.set_rot(b->get_relative_rotation());

		shader->set_uniform("M", m_transform.get_model() * tr.get_model());
		if (shader == renderer->color_shader)
		{
			shader->set_uniform("color", renderer->compute_selection_color());
			m_model->draw(shader, nullptr,false);
		}
		else
			m_model->draw(shader, nullptr);
	}
}

void ik_chain::draw_GUI()
{

}

vec3 ik_bone::get_relative_pos()
{
	if(m_parent == nullptr)
		return vec3{0.0f};

	vec3 p_pos = m_parent->get_relative_pos();
	mat4 p_rot = glm::mat4_cast(m_parent->m_rotation);
	mat4 p_tr = glm::translate(mat4(1.0f), vec3(m_parent->m_length, 0, 0));

	return {p_tr * p_rot * vec4(p_pos, 1.0f)};
}

quat ik_bone::get_relative_rotation()
{
	if (m_parent == nullptr)
		return m_rotation;
	return glm::normalize(m_parent->get_relative_rotation() * m_rotation);
}
