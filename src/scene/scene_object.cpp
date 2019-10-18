/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene_object.cpp
Purpose: Object base class
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/
#include "scene_object.h"
#include "graphics/renderer.h"
#include <imgui/imgui.h>
#include <platform/editor.h>
#include <platform/window.h>

scene_object::scene_object(std::string mesh, transform3d tr, animator * anim, curve_interpolator * curve_)
	: renderable(tr, renderer->get_model(mesh)), m_animator(anim), m_curve(curve_) {}

scene_object::~scene_object()
{
	if (m_animator) delete m_animator;
	if (m_curve) delete m_curve;
	if (editor->m_selected == this)
		editor->m_selected = nullptr;
}

void scene_object::update_parent_curve()
{
	if (m_curve->m_active)
	{
		vec3 pos = m_curve->m_curve->evaluate((float)m_curve->m_time);
		mat4 mat_pos = glm::translate(mat4(1.0), pos);

		double nxt_time = m_curve->m_time + 0.1;
		nxt_time = fmod(nxt_time, m_curve->m_curve->duration());
		vec3 nxt = m_curve->m_curve->evaluate((float)nxt_time);
		vec3 front = glm::normalize(pos-nxt);
		m_transform.set_rot(glm::normalize(glm::quatLookAt(front, vec3{ 0,1,0 })));
		m_transform.m_tr.parent = mat_pos;

		m_curve->update(m_curve->m_curve->duration());
	}
	else
		m_transform.m_tr.parent = mat4{1};
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

void interpolator::update(double max_t)
{
	if (!m_playback || m_playback_state)
		m_time += m_speed / window::frameTime;
	else
		m_time -= m_speed / window::frameTime;

	if (m_playback)
	{
		if (m_time >= max_t)
			m_time = 2 * max_t - m_time, m_playback_state = false;
		else if (m_time < 0.0f)
			m_time = -m_time, m_playback_state = true;
	}
	else
		m_time = fmod(m_time, max_t);
}

void interpolator::draw_GUI()
{
	ImGui::Checkbox("Active", &m_active);
	if (ImGui::Checkbox("Playback", &m_playback))m_playback_state = true;
	float sp = (float)m_speed;
	if (ImGui::SliderFloat("Speed", &sp, 0.01f, 10.0f)) m_speed = (double)sp;
	ImGui::Text(("Time:" + std::to_string(m_time)).c_str());
}
