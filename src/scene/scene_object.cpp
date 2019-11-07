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
	: renderable(tr, renderer->get_model(mesh)), m_animator(anim), m_curve_interpolator(curve_) {}

scene_object::~scene_object()
{
	if (m_animator) delete m_animator;
	if (m_curve_interpolator) delete m_curve_interpolator;
	if (editor->m_selected == this)
		editor->m_selected = nullptr;
}

void scene_object::update()
{
	if (m_curve_interpolator != nullptr)
	{
		if (m_curve_interpolator->m_active && m_curve_interpolator->m_actual_curve != nullptr)
		{
			float time = m_curve_interpolator->m_actual_curve->distance_to_time((float)m_curve_interpolator->m_time);
			vec3 pos = m_curve_interpolator->m_actual_curve->evaluate(time);
			mat4 mat_pos = glm::translate(mat4(1.0), pos);

			double nxt_dist = m_curve_interpolator->m_time + (m_curve_interpolator->m_playback_state?m_curve_interpolator->m_speed:-m_curve_interpolator->m_speed)/60.0f;
			nxt_dist = fmod(nxt_dist, m_curve_interpolator->m_actual_curve->max_distance());
			time = m_curve_interpolator->m_actual_curve->distance_to_time((float)nxt_dist);
			vec3 nxt = m_curve_interpolator->m_actual_curve->evaluate(time);
			vec3 front = pos - nxt;
			if (glm::length2(front) > glm::epsilon<float>())
			{
				front = glm::normalize(pos - nxt);
				m_transform.set_rot(glm::normalize(glm::quatLookAt(front, vec3{ 0,1,0 })));
			}
			m_transform.m_tr.parent = mat_pos;

			m_curve_interpolator->update(m_curve_interpolator->m_actual_curve->max_distance());
		}
		else
			m_transform.m_tr.parent = mat4{ 1 };
	}
	if (m_animator != nullptr)
	{
		if (m_animator->m_active && m_animator->m_current_animation != -1)
			if(m_model->m_animations.size() > m_animator->m_current_animation)
				m_animator->update(m_model->m_animations[m_animator->m_current_animation]->m_duration);
	}
}

void scene_object::draw_GUI()
{

	const char* current = m_model ? m_model->m_name.c_str() : "None";
	if (ImGui::BeginCombo("Mesh", current))
	{
		for (size_t n = 0; n < renderer->m_models.size(); n++)
		{
			bool is_selected = (m_model == renderer->m_models[n]);
			if (ImGui::Selectable(renderer->m_models[n]->m_name.c_str(), is_selected))
				m_model = renderer->m_models[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();

	}
	bool chng{ false };
	if (ImGui::DragFloat3("Position", &m_transform.m_tr.m_pos.x, .1f))chng = true;
	vec3 eu_angles = degrees(glm::eulerAngles(m_transform.m_tr.m_rot));
	if (ImGui::DragFloat3("Rotation", &eu_angles.x))
	{
		m_transform.m_tr.m_rot = normalize(quat(radians(eu_angles)));
		chng = true;
	}
	if (ImGui::DragFloat3("Scale", &m_transform.m_tr.m_scl.x, .1f, .001f, 99999999.f))chng = true;
	if (chng)m_transform.m_tr.upd();

	ImGui::NewLine();
	if (ImGui::TreeNode("Animator"))
	{
		if (m_animator)
		{
			m_animator->draw_GUI();
			if (ImGui::Button("Delete Animator"))
			{
				delete m_animator;
				m_animator = nullptr;
			}
		}
		else if (ImGui::Button("Create Animator"))
			m_animator = new animator;
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Curve Interpolator"))
	{
		if (m_curve_interpolator)
		{
			m_curve_interpolator->draw_GUI();
			if (ImGui::Button("Delete Curve Interpolator"))
			{
				delete m_curve_interpolator;
				m_curve_interpolator = nullptr;
			}
		}
		else if (ImGui::Button("Create Curve Interpolator"))
			m_curve_interpolator = new curve_interpolator;
		ImGui::TreePop();
	}
}

void scene_object::draw(Shader_Program * shader)
{
	m_transform.m_tr.save_prev();
	shader->set_uniform("M", m_transform.m_tr.get_model());

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
	if (ImGui::DragFloat("Speed", &sp, 0.01f, -1000.f,1000.f)) m_speed = (double)sp;
	ImGui::Text(("Time:" + std::to_string(m_time)).c_str());
}

void curve_interpolator::draw_GUI()
{
	std::string name = (m_actual_curve == nullptr) ? "None" : m_actual_curve->m_name;
	if (ImGui::BeginCombo("Curve", name.c_str()))
	{
		for (size_t n = 0; n < renderer->m_curves.size(); n++)
		{
			bool is_selected = m_actual_curve == renderer->m_curves[n];
			if (ImGui::Selectable(renderer->m_curves[n]->m_name.c_str(), is_selected))
				m_actual_curve = renderer->m_curves[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();

	}
	interpolator::draw_GUI();
}

void animator::draw_GUI()
{
	
	ImGui::SliderInt("AnimNum", &m_current_animation, -1, 5);
	interpolator::draw_GUI();
}
