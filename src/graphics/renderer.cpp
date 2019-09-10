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
	GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
	GL_CALL(glEnable(GL_CULL_FACE));

	//Load Programs
	try {
		color_shader = new Shader_Program("resources/shaders/basic.vert", "resources/shaders/color.frag");
		texture_shader = new Shader_Program("resources/shaders/basic.vert", "resources/shaders/texture.frag");
	}
	catch (const std::string & log) { std::cout << log; return false; }

	//Load Resources
	try
	{
		m_meshes.push_back(new Mesh("resources/meshes/cube.obj"));
		m_meshes.push_back(new Mesh("resources/meshes/octohedron.obj"));
		m_meshes.push_back(new Mesh("resources/meshes/quad.obj"));
		m_meshes.push_back(new Mesh("resources/meshes/segment.obj"));
		m_meshes.push_back(new Mesh("resources/meshes/sphere.obj"));
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
	GL_CALL(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));

	// Camera Update
	scene_cam.update();

	//color_shader->use();
	//color_shader->set_uniform("Model", glm::scale(mat4(1.0f), { 1.0f,0.25f, 1.0f }));
	//color_shader->set_uniform("VP", scene_cam.m_proj * scene_cam.m_view);
	//color_shader->set_uniform("doShadow", m_generator.m_shadowy);
	//// Draw Scene
	//GL_CALL(glEnable(GL_DEPTH_TEST));
	//GL_CALL(glBindVertexArray(m_generator.m_noise.m_naive_mesh.m_vao));
	//GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	//basic_shader->set_uniform("base_color", vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
	//GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_noise.m_naive_mesh.faces.size(), GL_UNSIGNED_INT, 0));
	//GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	//basic_shader->set_uniform("base_color", vec4{ 0.3f, 0.6f, 0.3f, 1.0f });
	//GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_noise.m_naive_mesh.faces.size(), GL_UNSIGNED_INT, 0));
	//GL_CALL(glDisable(GL_DEPTH_TEST));
}

void c_renderer::shutdown()
{
	//Clean Resouces

}
