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
	if (ImGui::DragFloat("Scale", &m_transform.m_tr.m_scl, .1f, .001f, 99999999.f))chng = true;
	if (chng)m_transform.m_tr.upd();
	ImGui::Checkbox("Tesselate", &m_tesselate);

	ImGui::NewLine();
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

	ImGui::NewLine();
	if (m_curve)
	{
		m_curve->draw_GUI();
		if (ImGui::Button("Delete Curve Interpolator"))
		{
			delete m_curve;
			m_curve = nullptr;
		}
	}
	else if (ImGui::Button("Create Curve Interpolator"))
		m_curve = new curve_interpolator;

	ImGui::NewLine();
}

void scene_object::update_parent_curve()
{
	if (m_curve->m_active && m_curve->m_actual_curve != nullptr)
	{
		vec3 pos = m_curve->m_actual_curve->evaluate((float)m_curve->m_time);
		mat4 mat_pos = glm::translate(mat4(1.0), pos);

		double nxt_time = m_curve->m_time + 0.1;
		nxt_time = fmod(nxt_time, m_curve->m_actual_curve->duration());
		vec3 nxt = m_curve->m_actual_curve->evaluate((float)nxt_time);
		vec3 front = glm::normalize(pos-nxt);

		m_transform.set_rot(glm::normalize(glm::quatLookAt(front, vec3{ 0,1,0 })));
		m_transform.m_tr.parent = mat_pos;

		m_curve->update(m_curve->m_actual_curve->duration());
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

void curve_interpolator::draw_GUI()
{
	if (ImGui::TreeNode("Curve"))
	{
		std::string name = (m_actual_curve == nullptr) ? "None" : m_actual_curve->m_name;
		if (ImGui::BeginCombo("Mesh", name.c_str()))
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
		ImGui::TreePop();
	}
}

void animator::draw_GUI()
{
	if (ImGui::TreeNode("Animator"))
	{
		ImGui::SliderInt("AnimNum", &m_current_animation, -1, 0);
		interpolator::draw_GUI();
		ImGui::TreePop();
	}
}
