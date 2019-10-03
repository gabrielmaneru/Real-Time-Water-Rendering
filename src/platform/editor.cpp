/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	editor.cpp
Purpose: Window
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "editor.h"
#include "window_manager.h"
#include "window.h"
#include <functional>
#include <graphics/renderer.h>
#include <scene/scene.h>
#include <scene/scene_object.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuizmo.h>
#include <GL/gl3w.h>
#include <graphics/gl_error.h>

c_editor* editor = new c_editor;

bool c_editor::init()
{
	// Start ImGui
	ImGui::CreateContext();
	if (!ImGui_ImplGlfw_InitForOpenGL(window_manager->m_window->m_window, true))
		return false;
	const char* glsl_version = "#version 440";
	if(!ImGui_ImplOpenGL3_Init(glsl_version))
		return false;

	// Set Own Style
	ImGui::StyleColorsLight();
	ImGui::GetStyle().FrameRounding = 12;
	ImGui::GetStyle().Colors[2] = ImVec4{ 1.0f, 1.0f, 1.0f, 0.6f };
	ImGui::GetStyle().Colors[10] = ImVec4{ 0.8f, 0.0f, 0.2f, 1.0f };
	ImGui::GetStyle().Colors[11] = ImVec4{ 0.8f, 0.0f, 0.2f, 1.0f };

	ImGuizmo::SetOrthographic(false);
	return true;
}

void c_editor::update()
{
	// Draw ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	draw_main_window();
	selector();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void c_editor::shutdown()
{
	// Exit ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void c_editor::draw_main_window()
{
	ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoMove);
	renderer->drawGUI();
	ImGui::End();

	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove);
	scene->drawGUI();
	ImGui::End();
}

void c_editor::draw_selected_window()
{
	static ImGuizmo::OPERATION m_operation{ ImGuizmo::TRANSLATE };
	static ImGuizmo::MODE m_mode{ ImGuizmo::WORLD };
	static bool m_snap{ false };

	ImGui::Begin("Object", nullptr);
	if (ImGui::RadioButton("Translate", m_operation == ImGuizmo::TRANSLATE))
		m_operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", m_operation == ImGuizmo::ROTATE))
		m_operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", m_operation == ImGuizmo::SCALE))
		m_operation = ImGuizmo::SCALE;

	ImGui::Checkbox("", &m_snap);
	ImGui::SameLine();
	static vec3 m_snap_step{1.0f};
	float m_cur_step;
	switch (m_operation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &m_snap_step.x);
		m_cur_step = m_snap_step.x;
		break;
	case ImGuizmo::ROTATE:
		ImGui::InputFloat("Angle Snap", &m_snap_step.y);
		m_cur_step = m_snap_step.y;
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &m_snap_step.z);
		m_cur_step = m_snap_step.z;
		break;
	}

	const char* current = m_selected->m_model ? m_selected->m_model->m_name.c_str() : "Unknown";
	if (ImGui::BeginCombo("Mesh", current))
	{
		for (size_t n = 0; n < renderer->m_models.size(); n++)
		{
			bool is_selected = (m_selected->m_model == renderer->m_models[n]);
			if (ImGui::Selectable(renderer->m_models[n]->m_name.c_str(), is_selected))
				m_selected->m_model = renderer->m_models[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();

	}
	bool chng{ false };
	if (ImGui::DragFloat3("Position", &m_selected->m_transform.m_tr.m_pos.x, .1f)) chng = true;
	if (ImGui::DragFloat3("Rotation", &m_selected->m_transform.m_tr.m_rot.x)) chng = true;
	if (ImGui::DragFloat("Scale", &m_selected->m_transform.m_tr.m_scl, .1f, .001f, 9999.f)) chng = true;
	if (chng) m_selected->m_transform.m_tr.upd();
	if (ImGui::Button("Delete"))
	{
		for (size_t i = 0; i < scene->m_objects.size(); i++)
			if (scene->m_objects[i] == m_selected)
			{
				scene->m_objects.erase(scene->m_objects.begin() + i);
				delete m_selected;
				m_selected = nullptr;
				ImGui::End();
				return;
			}
	}

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	mat4 model = m_selected->m_transform.get_model();
	ImGuizmo::BeginFrame();
	ImGuizmo::Manipulate(
		&renderer->scene_cam.m_view[0][0],
		&renderer->scene_cam.m_proj[0][0],
		m_operation, m_mode, &model[0][0],
		NULL, m_snap ? &m_cur_step : NULL);

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];

	ImGuizmo::DecomposeMatrixToComponents(&model[0][0], matrixTranslation, matrixRotation, matrixScale);
	switch (m_operation)
	{
	case ImGuizmo::TRANSLATE:
		m_selected->m_transform.m_tr.m_pos = vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
		break;
	case ImGuizmo::ROTATE:
		m_selected->m_transform.m_tr.m_rot = vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
		break;
	case ImGuizmo::SCALE:
		vec3 diff = vec3(matrixScale[0], matrixScale[1], matrixScale[2]) - vec3(m_selected->m_transform.m_tr.m_scl);
		if (glm::abs(diff.x) > .0f)
			m_selected->m_transform.m_tr.m_scl = matrixScale[0];
		break;
	}
	m_selected->m_transform.m_tr.upd();
	ImGui::End();
}

void c_editor::selector()
{
	if (m_selected)
		draw_selected_window();
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && window::mouse_but_left_triggered)
		select_object();
}

void c_editor::select_object()
{
	glFlush();
	glFinish();
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer->g_buffer.m_fbo);

	vec4 data;
	auto wpos = window_manager->m_window->get_mouse_pos();
	glReadPixels(wpos.first, window_manager->get_height() - wpos.second, 1, 1, GL_RGBA, GL_FLOAT, &data.x);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (data.a == 0.f)
	{
		m_selected = false;
		return;
	}

	float scalar;
	if		(data.r == 1.f && data.g < 1.f && data.b == 0.f)
		scalar = data.g;
	else if (data.r > 0.f && data.g == 1.f && data.b == 0.f)
		scalar = 60.f*(1.f-data.r) + 60.f;
	else if (data.r == 0.f && data.g == 1.f && data.b < 1.f)
		scalar = 60.f*data.b + 120.f;
	else if (data.r == 0.f && data.g > 0.f && data.b == 1.f)
		scalar = 60.f*(1.f-data.g) + 180.f;
	else if (data.r < 1.f && data.g == 0.f && data.b == 1.f)
		scalar = 60.f*data.r + 240.f;
	else if (data.r == 1.f && data.g == 0.f && data.b > 0.f)
		scalar = 60.0f*(1.f-data.b) + 300.f;

	scalar /= 360.f;
	scalar *= static_cast<float>(renderer->m_selection_calls.second);
	scalar = floorf(scalar + 0.5f);
	size_t idx = static_cast<size_t>(scalar);
	if (idx < scene->m_objects.size())
		m_selected = scene->m_objects[idx];
	else
		m_selected = scene->m_lights[idx - scene->m_objects.size()];
}
