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
#include <imgui/imgui.h>
#include <platform/window.h>

scene_object::scene_object(std::string mesh, transform3d tr, animator * anim, curve * curve_)
	: renderable(tr, renderer->get_model(mesh)), m_animator(anim), m_curve(curve_) {}

void scene_object::update_parent_curve()
{
	vec3 pos = m_curve->evaluate(m_curve_time);
	mat4 mat_pos = glm::translate(mat4(1.0), pos);
	m_transform.m_tr.parent = mat_pos;

	m_curve_time += 1.0f / window::frameTime;
	if (m_curve_time >= m_curve->duration())
		m_curve_time -= m_curve->duration();
}

void scene_object::draw(Shader_Program * shader)
{
	m_transform.m_tr.save_prev();
	shader->set_uniform("M", m_transform.m_tr.get_model());
	if (m_curve != nullptr)
		update_parent_curve();
	if(m_model != nullptr)
		m_model->draw(shader, m_animator);
}

void animator::draw_GUI()
{
	if(ImGui::TreeNode("Animator"))
	{
		ImGui::Checkbox("Active", &m_active);
		ImGui::Checkbox("Playback", &m_playback);
		float sp = m_speed;
		if(ImGui::SliderFloat("Speed", &sp, 0.01f, 10.0f)) m_speed = sp;
		ImGui::Text(("Time:"+std::to_string(m_time)).c_str());
		ImGui::TreePop();
	}
}
