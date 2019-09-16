
#include "editor.h"
#include "window_manager.h"
#include "window.h"
#include <functional>
#include <graphics/renderer.h>
#include <scene/scene.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

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
	std::function<void(c_renderer::e_texture)> show_image = [&](c_renderer::e_texture txt)
	{
		const float scale = 2.0f;
		const ImVec2 rect{ scale*192.f, scale*108.f };
		GLuint id = renderer->get_texture(txt);
		ImGui::Image(*reinterpret_cast<ImTextureID*>(&id), rect, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f });
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			renderer->set_texture(txt);
	};

	ImGui::Begin("Base", nullptr, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos(ImVec2{ 0.0f, 0.0f });

	ImGui::SliderFloat("Near", &renderer->scene_cam.m_near, 0.001f, renderer->scene_cam.m_far);
	ImGui::SliderFloat("Far", &renderer->scene_cam.m_far, renderer->scene_cam.m_near, 1000.f);
	ImGui::SliderInt("Test", &m_test_var, 0, 10);

	show_image(c_renderer::DIFFUSE);
	show_image(c_renderer::POSITION);
	show_image(c_renderer::NORMAL);
	show_image(c_renderer::LIGHT);

	bool chng{ false };
	if (ImGui::InputFloat3("Pos", &scene->m_objects[1]->m_transform.m_tr.m_pos.x))chng = true;
	if (ImGui::InputFloat3("Scl", &scene->m_objects[1]->m_transform.m_tr.m_scl.x))chng = true;
	if (chng)scene->m_objects[1]->m_transform.m_tr.upd();
	ImGui::End();
}