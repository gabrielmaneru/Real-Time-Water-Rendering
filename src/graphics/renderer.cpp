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

	/*
	switch (m_generator.step)
	{
	case s_select_noise_map:
		{// Draw Mesh
			m = glm::scale(mat4(1.0f), { 1.0f,0.25f, 1.0f });
			vp = scene_cam.m_proj * scene_cam.m_view;
			basic_shader->use();
			basic_shader->set_uniform("Model", m);
			basic_shader->set_uniform("VP", vp);
			basic_shader->set_uniform("doShadow", m_generator.m_shadowy);
			// Draw Scene
			GL_CALL(glEnable(GL_DEPTH_TEST));
			GL_CALL(glBindVertexArray(m_generator.m_noise.m_naive_mesh.m_vao));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
			basic_shader->set_uniform("base_color", vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_noise.m_naive_mesh.faces.size(), GL_UNSIGNED_INT, 0));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			basic_shader->set_uniform("base_color", vec4{ 0.3f, 0.6f, 0.3f, 1.0f });
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_noise.m_naive_mesh.faces.size(), GL_UNSIGNED_INT, 0));
			GL_CALL(glDisable(GL_DEPTH_TEST));
		}
		{// Draw Noise Image
			GL_CALL(glViewport((GLsizei)(window_manager->get_width()*(4.0f / 5.0f)), 0, (GLsizei)(window_manager->get_width() / 5.0f), (GLsizei)(window_manager->get_height() / 5.0f)));
			
			texture_shader->use();
			texture_shader->set_uniform("MVP", ortho_cam.m_proj * ortho_cam.m_view);
			texture_shader->set_uniform("dim", 1);
			GL_CALL(glActiveTexture(GL_TEXTURE0));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, m_generator.m_noise.m_texture.m_id));
			
			// Draw Quad
			m_meshes[quad]->use();
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, m_meshes[quad]->idx_count(), GL_UNSIGNED_SHORT, 0));
		}
		break;

	case s_apply_layers:
		{// Draw Terrain
			vp = scene_cam.m_proj * scene_cam.m_view;
			layer_shader->use();
			layer_shader->set_uniform("MVP", vp);
			m_generator.set_uniforms(layer_shader, generator::e_shader::e_layer_color);

			// Draw Scene
			GL_CALL(glEnable(GL_DEPTH_TEST));
			GL_CALL(glBindVertexArray(m_generator.m_layered_mesh.m_vao));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_layered_mesh.faces.size(), GL_UNSIGNED_INT, 0));

			// Draw Water
			water_shader->use();
			water_shader->set_uniform("useColor", true);
			water_shader->set_uniform("VP", vp);
			m_generator.set_uniforms(water_shader, generator::e_shader::e_water);

			m_meshes[quad]->use();
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, m_meshes[quad]->idx_count(), GL_UNSIGNED_SHORT, 0));

			// Draw Terrain Mesh
			layer_shader->use();
			layer_shader->set_uniform("MVP", vp);
			m_generator.set_uniforms(layer_shader, generator::e_shader::e_layer_mesh);
			GL_CALL(glBindVertexArray(m_generator.m_layered_mesh.m_vao));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_layered_mesh.faces.size(), GL_UNSIGNED_INT, 0));
			GL_CALL(glDisable(GL_DEPTH_TEST));
		}
		break;

	case s_rasterization:
	{// Draw Mesh
		vp = scene_cam.m_proj * scene_cam.m_view;
		gradient_shader->use();
		gradient_shader->set_uniform("Model", mat4(1.0f));
		gradient_shader->set_uniform("VP", vp);
		gradient_shader->set_uniform("dim", 3);
		gradient_shader->set_uniform("doClip", true);
		gradient_shader->set_uniform("doShading", m_generator.m_eroder.m_display_mode != eroder::tracer);
		gradient_shader->set_uniform("clip_normal", vec4(0.0f, 1.0f, 0.0f, -m_generator.m_water_height));
		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, m_generator.m_rasterized.texture.m_id));
		
		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glEnable(GL_CLIP_DISTANCE0));

		// Draw Scene
		GL_CALL(glBindVertexArray(m_generator.m_rasterized.mesh.m_vao));
		GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_rasterized.mesh.faces.size(), GL_UNSIGNED_INT, 0));


		// Draw Water
		{
			// Bind Refraction
			GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_generator.m_refraction.m_fbo));
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			GL_CALL(glViewport(0, 0, m_generator.m_refraction.m_width, m_generator.m_refraction.m_height));
			// Draw Refraction
			gradient_shader->set_uniform("clip_normal", vec4(0.0f, -1.0f, 0.0f, m_generator.m_water_height));
			GL_CALL(glBindVertexArray(m_generator.m_rasterized.mesh.m_vao));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_rasterized.mesh.faces.size(), GL_UNSIGNED_INT, 0));
			gradient_shader->set_uniform("VP", vp);
			// Bind Reflection
			GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_generator.m_reflection.m_fbo));
			GL_CALL(glClearColor(0.8f, 0.7f, 0.7f, 1.0f));
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			GL_CALL(glViewport(0, 0, m_generator.m_reflection.m_width, m_generator.m_reflection.m_height));
			// Move Camera
			gradient_shader->set_uniform("VP", scene_cam.m_proj * invert_cam.m_view);
			// Draw Reflection
			gradient_shader->set_uniform("clip_normal", vec4(0.0f, 1.0f, 0.0f, -m_generator.m_water_height));
			GL_CALL(glBindVertexArray(m_generator.m_rasterized.mesh.m_vao));
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_rasterized.mesh.faces.size(), GL_UNSIGNED_INT, 0));
			GL_CALL(glDisable(GL_CLIP_DISTANCE0));


			GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));
			water_shader->use();
			water_shader->set_uniform("VP", vp);
			water_shader->set_uniform("useColor", false);
			water_shader->set_uniform("base_color", m_generator.m_wcolor);
			water_shader->set_uniform("reflect_factor", m_generator.m_reflect_factor);
			water_shader->set_uniform("color_factor", m_generator.m_wcolor_factor);
			water_shader->set_uniform("win_width", (float)window_manager->get_width());
			water_shader->set_uniform("win_height", (float)window_manager->get_height());
			water_shader->set_uniform_sampler("reflection_texture", 0);
			water_shader->set_uniform_sampler("refraction_texture", 1);
			m_generator.set_uniforms(water_shader, generator::e_shader::e_water);
			GL_CALL(glActiveTexture(GL_TEXTURE0));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, m_generator.m_reflection.m_color_texture))
			GL_CALL(glActiveTexture(GL_TEXTURE1));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, m_generator.m_refraction.m_color_texture));
			m_meshes[quad]->use();
			GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			GL_CALL(glDrawElements(GL_TRIANGLES, m_meshes[quad]->idx_count(), GL_UNSIGNED_SHORT, 0));
		}

		// Draw Terrain Mesh
		basic_shader->use();
		basic_shader->set_uniform("Model", mat4(1.0f));
		basic_shader->set_uniform("VP", vp);
		basic_shader->set_uniform("base_color", vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		GL_CALL(glBindVertexArray(m_generator.m_rasterized.mesh.m_vao));
		GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
		GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_generator.m_rasterized.mesh.faces.size(), GL_UNSIGNED_INT, 0));
		GL_CALL(glDisable(GL_DEPTH_TEST));
	}
		break;
	default:
		break;
	}
	*/
}

void c_renderer::shutdown()
{
	//Clean Resouces

}
