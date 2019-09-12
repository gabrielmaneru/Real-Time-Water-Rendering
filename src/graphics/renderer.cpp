#include "renderer.h"
#include "gl_error.h"
#include <platform/window_manager.h>
#include <platform/window.h>
#include <scene/scene.h>
#include <utils/generate_noise.h>
#include <GL/gl3w.h>
#include <iostream>
#include <algorithm>

c_renderer* renderer = new c_renderer;

bool c_renderer::init()
{
	if (gl3wInit())
		return false;
	
	if (!gl3wIsSupported(4, 0))
		return false;

	// GL Options
	GL_CALL(glEnable(GL_DEPTH_TEST));

	// Load Programs
	try {
		g_buffer_shader = new Shader_Program("../data/shaders/basic.vert", "../data/shaders/g_buffer.frag");
		light_shader = new Shader_Program("../data/shaders/basic.vert", "../data/shaders/light.frag");
		texture_shader = new Shader_Program("../data/shaders/basic.vert", "../data/shaders/texture.frag");
	}
	catch (const std::string & log) { std::cout << log; return false; }

	// Load Resources
	try
	{
		// Basic
		m_models.push_back(new Model("../data/meshes/cube.obj"));
		m_models.push_back(new Model("../data/meshes/octohedron.obj"));
		m_models.push_back(new Model("../data/meshes/quad.obj"));
		m_models.push_back(new Model("../data/meshes/sphere.obj"));

		// Complex
		m_models.push_back(new Model("../data/meshes/sponza.obj"));
	}
	catch (const std::string & log) { std::cout << log; return false; }

	// Setup Cameras
	scene_cam.update();
	ortho_cam.m_eye = { 0.0f, 0.0f, 1.0f };
	ortho_cam.m_front = { 0.0f, 0.0f, -1.0f };
	ortho_cam.update();

	// Setup Framebuffers
	g_buffer.setup(window_manager->get_width(), window_manager->get_height(), 3u);
	light_buffer.setup(window_manager->get_width(), window_manager->get_height(), 1u);
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
	/**/scene->draw(g_buffer_shader);
	///////////////////////////////////////////////////////////////////////////



	// Light Pass	///////////////////////////////////////////////////////////
	/**///GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, light_buffer.m_fbo));
	/**///GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	/**///GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	/**///GL_CALL(glViewport(0, 0, light_buffer.m_width, light_buffer.m_height));
	/**///
	/**///light_shader->use();
	/**///light_shader->set_uniform("P", ortho_cam.m_proj);
	/**///light_shader->set_uniform("V", ortho_cam.m_view);
	/**///light_shader->set_uniform("M", mat4(1.0f));
	/**///light_shader->set_uniform("light_position", scene->m_objects[1]->m_transform.get_pos());
	/**///light_shader->set_uniform("la", vec3{ 0.1, 0.1, 0.1 });
	/**///light_shader->set_uniform("ld", vec3{ 0.8, 0.8, 0.8 });
	/**///light_shader->set_uniform("ls", vec3{ 1.0, 1.0, 1.0 });
	/**///
	/**///light_shader->set_uniform_sampler("diffuse_txt", 0);
	/**///glActiveTexture(GL_TEXTURE0);
	/**///glBindTexture(GL_TEXTURE_2D, g_buffer.m_color_texture[0]);
	/**///
	/**///light_shader->set_uniform_sampler("position_txt", 1);
	/**///glActiveTexture(GL_TEXTURE1);
	/**///glBindTexture(GL_TEXTURE_2D, g_buffer.m_color_texture[1]);
	/**///
	/**///light_shader->set_uniform_sampler("normal_txt", 2);
	/**///glActiveTexture(GL_TEXTURE2);
	/**///glBindTexture(GL_TEXTURE_2D, g_buffer.m_color_texture[2]);
	/**///
	/**///m_models[2]->m_meshes[0]->draw(light_shader);
	///////////////////////////////////////////////////////////////////////////



	// Last Render Pass	///////////////////////////////////////////////////////
	/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	/**/GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	/**/GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));
	/**/
	/**/texture_shader->use();
	/**/texture_shader->set_uniform("P", ortho_cam.m_proj);
	/**/texture_shader->set_uniform("V", ortho_cam.m_view);
	/**/texture_shader->set_uniform("M", mat4(1.0f));
	/**/
	/**/texture_shader->set_uniform_sampler("uniform_texture", 0);
	/**/glActiveTexture(GL_TEXTURE0);
	/**/glBindTexture(GL_TEXTURE_2D, g_buffer.m_color_texture[2]);
	/**/
	/**/m_models[2]->m_meshes[0]->draw(texture_shader);
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

const Model * c_renderer::get_model(std::string s)
{
	for (auto& model : m_models)
		if (model->m_name == s)
			return model;
	return nullptr;
}
