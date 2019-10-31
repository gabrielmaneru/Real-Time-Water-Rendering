/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	light.cpp
Purpose: Light base class
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "light.h"
#include <graphics/renderer.h>
#include <graphics/transform3d.h>
#include <imgui/imgui.h>

vec3 light_data::m_ambient = vec3{ 0.0f };
float light_data::m_att_max{ 0.01f };

void light_data::drawGUI()
{
	static bool display_break{ false };

	ImGui::Checkbox("", &display_break);
	ImGui::SameLine();
	if (display_break)
		ImGui::DragFloat3("Diff", &m_diffuse.x, 0.01f, 0.0f, 1000.0f);
	else if (ImGui::DragFloat("Diff", &m_diffuse.x, 0.01f, 0.0f, 1000.0f))
		m_diffuse.y = m_diffuse.z = m_diffuse.x;

	ImGui::DragFloat3("Att", &m_att_factor.x, 0.0001f, 0.001f, 1.0f);
}

void light::recompute_scale()
{
	// Compute radius of max att
	const vec3& att = m_ldata.m_att_factor;
	float a = m_ldata.m_att_max*m_ldata.m_att_factor.z;
	float b = m_ldata.m_att_max*m_ldata.m_att_factor.y;
	float c = m_ldata.m_att_max*m_ldata.m_att_factor.x - 1;

	float det = b*b-4*a*c;
	float r1 = (-b + sqrtf(det)) / (2 * a);
	m_transform.set_scl(vec3(r1));
}

light::light(transform3d tr, light_data ld)
	:scene_object("sphere", tr, nullptr, nullptr), m_ldata(ld)
{
}

void light::draw(Shader_Program * shader)
{
	shader->set_uniform("l_pos", vec3(renderer->scene_cam.m_view * vec4(m_transform.get_pos(), 1.0f)));
	shader->set_uniform("ld", m_ldata.m_diffuse);
	shader->set_uniform("att_factor", m_ldata.m_att_factor);
	shader->set_uniform("rad", m_transform.m_tr.m_scl.x);

	if (m_model)
	{
		recompute_scale();
		shader->set_uniform("M", m_transform.get_model());
		m_model->draw(shader,nullptr, false);
	}
}
