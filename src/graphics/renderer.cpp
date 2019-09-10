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
	GL_CALL(glClearColor(0.5f, 0.0f, 0.0f, 1.0f));
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDisable(GL_BLEND));

	//Load Programs
	try {
		color_shader = new Shader_Program("../data/shaders/basic.vert", "../data/shaders/color.frag");
		texture_shader = new Shader_Program("../data/shaders/basic.vert", "../data/shaders/g_buffer.frag");
	}
	catch (const std::string & log) { std::cout << log; return false; }

	//Load Resources
	try
	{
		// Basic
		m_models.push_back(new Model("../data/meshes/cube.obj"));
		//m_models.push_back(new Model("../data/meshes/octohedron.obj"));
		//m_models.push_back(new Model("../data/meshes/quad.obj"));
		//m_models.push_back(new Model("../data/meshes/segment.obj"));
		//m_models.push_back(new Model("../data/meshes/sphere.obj"));

		// Complex
		//m_models.push_back(new Model("../data/meshes/sponza.obj"));
	}
	catch (const std::string & log) { std::cout << log; return false; }


	// Setup Cameras
	scene_cam.update();
	ortho_cam.update();

	return true;
}

void c_renderer::update()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));

	// Camera Update
	scene_cam.update();

	texture_shader->use();
	texture_shader->set_uniform("VP", scene_cam.m_proj*scene_cam.m_view);
	scene->draw(texture_shader);
	

}

void c_renderer::shutdown()
{
	//Clean Resouces

}
