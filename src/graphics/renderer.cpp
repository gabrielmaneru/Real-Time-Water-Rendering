#include "renderer.h"
#include "gl_error.h"
#include <platform/window_manager.h>
#include <platform/window.h>
#include <platform/editor.h>
#include <scene/scene.h>
#include <utils/generate_noise.h>
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <iostream>
#include <algorithm>

c_renderer* renderer = new c_renderer;

bool c_renderer::init()
{
	if (gl3wInit())
		return false;
	
	if (!gl3wIsSupported(4, 6))
		return false;

	// GL Options
	setup_gl_debug();
	glCullFace(GL_FRONT);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	// Load Programs
	try {
		g_buffer_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/g_buffer.frag");
		light_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/light.frag");
		texture_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/texture.frag");
	}
	catch (const std::string & log) { std::cout << log; return false; }

	// Load Resources
	try
	{
		// Basic
		m_models.push_back(new Model("./data/meshes/cube.obj"));
		m_models.push_back(new Model("./data/meshes/octohedron.obj"));
		m_models.push_back(new Model("./data/meshes/quad.obj"));
		m_models.push_back(new Model("./data/meshes/sphere.obj"));

		// Complex
		m_models.push_back(new Model("./data/meshes/sponza.obj"));
		m_models.push_back(new Model("./data/meshes/phoenix.fbx"));
	}
	catch (const std::string & log) { std::cout << log; return false; }
	
	// Setup Cameras
	scene_cam.m_eye = { 4,16,44 };
	scene_cam.update();

	// Setup Framebuffers
	g_buffer.setup(window_manager->get_width(), window_manager->get_height(),{
		GL_RGBA16F, GL_RGBA,
		GL_RGBA16F, GL_RGBA,
		GL_RGBA16F, GL_RGBA,
		});
	light_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
		GL_RGBA16F, GL_RGBA
		},g_buffer.m_depth_texture);
	return true;
}

void c_renderer::update()
{
	// Camera Update
	scene_cam.update();

	// G_Buffer Pass	///////////////////////////////////////////////////////
	/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.m_fbo));
	/**/GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	/**/GL_CALL(glViewport(0, 0, g_buffer.m_width, g_buffer.m_height));
	/**/
	/**/g_buffer_shader->use();
	/**/g_buffer_shader->set_uniform("P", scene_cam.m_proj);
	/**/g_buffer_shader->set_uniform("V", scene_cam.m_view);
	/**/GL_CALL(glEnable(GL_DEPTH_TEST));
	/**/scene->draw_objs(g_buffer_shader);
	/**/if (m_render_options.render_lights)
	/**/	scene->draw_debug_lights(g_buffer_shader);
	/**/GL_CALL(glDisable(GL_DEPTH_TEST));
	///////////////////////////////////////////////////////////////////////////



	// Light Pass	///////////////////////////////////////////////////////////
	/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, light_buffer.m_fbo));
	/**/GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
	/**/GL_CALL(glViewport(0, 0, light_buffer.m_width, light_buffer.m_height));
	/**/glDepthMask(GL_FALSE);
	/**/
	/**/light_shader->use();
	/**/// Render Ambient
	/**/light_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "render_ambient");
	/**/light_shader->set_uniform("P", mat4(1.0f));
	/**/light_shader->set_uniform("V", mat4(1.0f));
	/**/light_shader->set_uniform("M", mat4(1.0f));
	/**/glActiveTexture(GL_TEXTURE0);
	/**/light_shader->set_uniform_sampler(0);
	/**/glBindTexture(GL_TEXTURE_2D, get_texture(DIFFUSE));
	/**/glActiveTexture(GL_TEXTURE1);
	/**/light_shader->set_uniform_sampler(1);
	/**/glBindTexture(GL_TEXTURE_2D, get_texture(POSITION));
	/**/glActiveTexture(GL_TEXTURE2);
	/**/light_shader->set_uniform_sampler(2);
	/**/glBindTexture(GL_TEXTURE_2D, get_texture(NORMAL));
	/**/m_models[2]->m_meshes[0]->draw(light_shader);
	/**/
	/**/// Render Lights
	/**/light_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "render_diffuse_specular");
	/**/light_shader->set_uniform("P", scene_cam.m_proj);
	/**/light_shader->set_uniform("V", scene_cam.m_view);
	/**/light_shader->set_uniform("window_width", window_manager->get_width());
	/**/light_shader->set_uniform("window_height", window_manager->get_height());
	/**/GL_CALL(glEnable(GL_BLEND));
	/**/GL_CALL(glEnable(GL_DEPTH_TEST));
	/**/glDepthFunc(GL_GREATER);
	/**/glEnable(GL_CULL_FACE);
	/**/scene->draw_lights(light_shader);
	/**/glDisable(GL_CULL_FACE);
	/**/glDepthFunc(GL_LESS);
	/**/GL_CALL(glDisable(GL_DEPTH_TEST));
	/**/GL_CALL(glDepthMask(GL_TRUE));
	/**/GL_CALL(glDisable(GL_BLEND));
	///////////////////////////////////////////////////////////////////////////



	// Last Render Pass	///////////////////////////////////////////////////////
	/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	/**/GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	/**/GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));
	/**/
	/**/texture_shader->use();
	/**/texture_shader->set_uniform("P", mat4(1.0f));
	/**/texture_shader->set_uniform("V", mat4(1.0f));
	/**/texture_shader->set_uniform("M", mat4(1.0f));
	/**/
	/**/texture_shader->set_uniform_sampler(0);
	/**/glActiveTexture(GL_TEXTURE0);
	/**/glBindTexture(GL_TEXTURE_2D, get_texture(m_txt_cur));
	/**/
	/**/GL_CALL(glEnable(GL_DEPTH_TEST));
	/**/m_models[2]->m_meshes[0]->draw(texture_shader);
	/**/GL_CALL(glDisable(GL_DEPTH_TEST));
	///////////////////////////////////////////////////////////////////////////
}

void c_renderer::shutdown()
{
	delete color_shader;
	delete g_buffer_shader;

	// Clean Meshes
	for (auto m : m_models)
		delete m;
	m_models.clear();
}

void c_renderer::drawGUI()
{
	if (ImGui::TreeNode("Camera"))
	{
		ImGui::SliderFloat("Near", &renderer->scene_cam.m_near, 0.001f, renderer->scene_cam.m_far);
		ImGui::SliderFloat("Far", &renderer->scene_cam.m_far, renderer->scene_cam.m_near, 1000.f);
		ImGui::SliderFloat("Fov", &renderer->scene_cam.m_fov, 30.0f, 150.f);
		if (auto target = scene_cam.get_target())
		{
			std::string name = target->m_model
				? target->m_model->m_name
				: "Unknown";
			ImGui::Text(("Target: "+name).c_str());
			ImGui::SameLine();
			if (ImGui::Button("Release"))
				scene_cam.release_target();
		}
		else
		{
			if (ImGui::BeginCombo("Target", "Select Obj"))
			{
				for (size_t n = 0; n < scene->m_objects.size(); n++)
				{
					if (ImGui::Selectable(scene->m_objects[n]->m_model->m_name.c_str(), false))
						scene_cam.use_target(scene->m_objects[n]);
				}
				ImGui::EndCombo();
			}
		}
		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Deferred Buffers"))
	{
		std::function<void(c_renderer::e_texture)> show_image = [&](c_renderer::e_texture txt)
		{
			const float scale = 2.0f;
			const ImVec2 rect{ scale*128.f, scale*72.f };
			GLuint id = renderer->get_texture(txt);
			ImGui::Image(*reinterpret_cast<ImTextureID*>(&id), rect, ImVec2{ 0.f, 1.f }, ImVec2{ 1.f, 0.f });
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				renderer->set_texture(txt);
		};
		show_image(c_renderer::DIFFUSE);
		show_image(c_renderer::POSITION);
		show_image(c_renderer::NORMAL);
		show_image(c_renderer::DEPTH);
		show_image(c_renderer::LIGHT);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("RenderOptions"))
	{
		ImGui::Checkbox("Render Lights", &m_render_options.render_lights);
		ImGui::TreePop();
	}
}

GLuint c_renderer::get_texture(e_texture ref)
{
	switch (ref)
	{
	case c_renderer::e_texture::DIFFUSE:
		return g_buffer.m_color_texture[0];
	case c_renderer::e_texture::POSITION:
		return g_buffer.m_color_texture[1];
	case c_renderer::e_texture::NORMAL:
		return g_buffer.m_color_texture[2];
	case c_renderer::e_texture::DEPTH:
		return g_buffer.m_depth_texture;
	case c_renderer::e_texture::LIGHT:
		return light_buffer.m_color_texture[0];
	}
	return 0;
}

const Model * c_renderer::get_model(std::string s)
{
	for (auto& model : m_models)
		if (model->m_name == s)
			return model;
	return nullptr;
}
