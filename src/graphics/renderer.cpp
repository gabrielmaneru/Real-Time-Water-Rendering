/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	renderer.cpp
Purpose: OpenGl renderer
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

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

void c_renderer::update_max_draw_call_count()
{
	m_selection_calls = { 0u, scene->m_objects.size() };
	if (m_render_options.render_lights)
		m_selection_calls.second += scene->m_point_lights.size();
}

vec3 c_renderer::compute_selection_color()
{
	float scalar = static_cast<float>(m_selection_calls.first++) / static_cast<float>(m_selection_calls.second)*360.f;
	float val = fmod(scalar, 60.f)/60.f;

	vec3 color;
	if		(scalar < 60.f)
		color = { 1.0f, val, 0.0f };
	else if (scalar < 120.f)
		color = { 1.f-val, 1.0f, 0.0f };
	else if (scalar < 180.f)
		color = { 0.0f, 1.f, val };
	else if (scalar < 240.f)
		color = { 0.0f, 1.f-val, 1.0f};
	else if (scalar < 300.f)
		color = { val, 0.0f, 1.0f };
	else
		color = { 1.0f, 0.0f, 1.f-val };
	return color;
}

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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Load Programs
	try {
		g_buffer_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/g_buffer.frag");
		decal_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/decal.frag");
		light_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/light.frag");
		ao_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/hbao.frag");
		blur_shader	= new Shader_Program("./data/shaders/basic.vert", "./data/shaders/blur.frag");
		texture_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/texture.frag");
		color_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/color.frag");
		skybox_shader = new Shader_Program("./data/shaders/basic.vert", "./data/shaders/skybox.frag");
	}
	catch (const std::string & log) { std::cout << log; }

	// Materials
	Model::m_def_materials.push_back(new Material{ "default",
		{0.0,1.0,0.0},{},
		vec3{1.0},{},
		{1.0f},{},
		1.0f,{}
		});
	Model::m_def_materials.push_back(new Material{ "plastic",
		{},"plasticpattern1-albedo.png",
		{},"Dielectric_metallic.tga",
		{},"plasticpattern1-roughness2.png",
		1.0f,"plasticpattern1-normal2b.png"
		});
	Model::m_def_materials.push_back(new Material{ "copper",
		{},"plasticpattern1-albedo.png",
		{},"oxidized-copper-metal.png",
		{},"oxidized-coppper-roughness.png",
		1.0f,"oxidized-copper-normal-ue.png"
		});

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
	}
	catch (const std::string & log) { std::cout << log; return false; }
	
	// Curves
	m_curves.push_back(new curve_catmull("walk"));
	m_curves.push_back(new curve_catmull("swim"));
	m_curves.push_back(new curve_catmull("swim2"));
	
	// Setup Cameras
	scene_cam.m_eye = { 29,16,-4 };
	scene_cam.update();

	randomize_noise();
	m_render_options.ao_noise = generate_noise(512, 16.0f, 4, 2.0f, 2.0f);
	m_render_options.ao_noise.load();

	skybox.loadCubemapFromFile({
		Texture::filter_name("px.png").c_str(),
		Texture::filter_name("nx.png").c_str(),
		Texture::filter_name("py.png").c_str(),
		Texture::filter_name("ny.png").c_str(),
		Texture::filter_name("pz.png").c_str(),
		Texture::filter_name("nz.png").c_str(),
	});
	return true;
}

void c_renderer::update()
{
	// Setup Framebuffers
	if (window_manager->get_width() != g_buffer.m_width
	||  window_manager->get_height() != g_buffer.m_height)
	{
		g_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST,
			GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST,
			GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST,
			GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST,
			GL_R16F, GL_RED, GL_FLOAT, GL_NEAREST
			});
		selection_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST
			});
		ao_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_R16F, GL_RED, GL_FLOAT, GL_LINEAR
			});
		light_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_NEAREST
			}, g_buffer.m_depth_texture);
		blur_control_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR
			});
		bloom_buffer.setup(window_manager->get_width()/2, window_manager->get_height()/2, {
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR,
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR
			});
		blur_buffer.setup(window_manager->get_width(), window_manager->get_height(), {
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR,
			GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR
			});
	}

	// Camera Update
	scene_cam.save_prev();
	scene_cam.update();

	if (g_buffer_shader->is_valid())
	{
		// G_Buffer Pass	///////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, g_buffer.m_width, g_buffer.m_height));
		/**/GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/
		/**/g_buffer_shader->use();
		/**/scene_cam.set_uniforms(g_buffer_shader);
		/**/g_buffer_shader->set_uniform("near", scene_cam.m_near);
		/**/g_buffer_shader->set_uniform("far", scene_cam.m_far);
		/**/scene->draw_objs(g_buffer_shader);
		/**/
		/**/if(m_render_options.dc_active)
		/**/{
		/**/	if (m_render_options.dc_mode == 0)
		/**/		GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/	decal_shader->use();
		/**/	scene_cam.set_uniforms(decal_shader);
		/**/	decal_shader->set_uniform("width", (float)g_buffer.m_width);
		/**/	decal_shader->set_uniform("height", (float)g_buffer.m_height);
		/**/	decal_shader->set_uniform("angle", m_render_options.dc_angle);
		/**/	decal_shader->set_uniform("mode", m_render_options.dc_mode);
		/**/	g_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0+1, GL_COLOR_ATTACHMENT0+3 });
		/**/	glActiveTexture(GL_TEXTURE2);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(DEPTH));
		/**/	scene->draw_decals(decal_shader);
		/**/	g_buffer.set_drawbuffers();
		/**/	if(m_render_options.dc_mode==0)
		/**/		GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/}
		/**/
		/**/g_buffer_shader->use();
		/**/scene_cam.set_uniforms(g_buffer_shader);
		/**/g_buffer_shader->set_uniform("near", scene_cam.m_near);
		/**/g_buffer_shader->set_uniform("far", scene_cam.m_far);
		/**/if (m_render_options.render_lights)
		/**/	scene->draw_debug_lights(g_buffer_shader);
		/**/if (m_render_options.render_curves)
		/**/	scene->draw_debug_curves(g_buffer_shader);
		/**/GL_CALL(glDisable(GL_DEPTH_TEST));
		///////////////////////////////////////////////////////////////////////////
	}
	if (color_shader->is_valid())
	{
		// Selection Pass	///////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, selection_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, selection_buffer.m_width, selection_buffer.m_height));
		/**/color_shader->use();
		/**/scene_cam.set_uniforms(color_shader);
		/**/scene_cam.set_prev_uniforms(color_shader);
		/**/color_shader->set_uniform("mb_camera_motion", m_render_options.mb_camera_blur);
		/**/GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/update_max_draw_call_count();
		/**/for (auto p_obj : scene->m_objects)
			/**/ {
			/**/	color_shader->set_uniform("M", p_obj->m_transform.get_model());
			/**/	color_shader->set_uniform("M_prev", p_obj->m_transform.m_tr.get_prev_model());
			/**/	color_shader->set_uniform("color", compute_selection_color());
			/**/	if (p_obj->m_model != nullptr)
			/**/		p_obj->m_model->draw(color_shader, p_obj->m_animator, false);
			/**/
		}
		/**/if (m_render_options.render_lights)
			/**/	scene->draw_debug_lights(color_shader);
		/**/GL_CALL(glDisable(GL_DEPTH_TEST));
		///////////////////////////////////////////////////////////////////////////
	}
	if (ao_shader->is_valid())
	{
		// AO Pass	///////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, ao_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, ao_buffer.m_width, ao_buffer.m_height));
		/**/ao_shader->use();
		/**/ortho_cam.set_uniforms(ao_shader);
		/**/ao_shader->set_uniform("width", (float)ao_buffer.m_width);
		/**/ao_shader->set_uniform("noise", false);
		/**/ao_shader->set_uniform("random_offset", vec2(random_float(-0.5f,0.5), random_float(-0.5, 0.5)));
		/**/ao_shader->set_uniform("height", (float)ao_buffer.m_height);
		/**/ao_shader->set_uniform("radius", m_render_options.ao_radius);
		/**/ao_shader->set_uniform("bias", m_render_options.ao_angle_bias);
		/**/ao_shader->set_uniform("num_dirs", m_render_options.ao_num_dirs);
		/**/ao_shader->set_uniform("num_steps", m_render_options.ao_num_steps);
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(POSITION));
		/**/glActiveTexture(GL_TEXTURE1);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(NORMAL));
		/**/glActiveTexture(GL_TEXTURE2);
		/**/glBindTexture(GL_TEXTURE_2D, m_render_options.ao_noise.m_id);
		/**/m_models[2]->m_meshes[0]->draw(ao_shader);
		///////////////////////////////////////////////////////////////////////////
	}
	if (light_shader->is_valid() && skybox_shader->is_valid())
	{
		// Light Pass	///////////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, light_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, light_buffer.m_width, light_buffer.m_height));
		/**/glDepthMask(GL_FALSE);
		/**/light_shader->use();
		/**/
		/**/// Render Ambient
		/**/light_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "render_ambient");
		/**/ortho_cam.set_uniforms(light_shader);
		/**/if (scene->m_dir_light) scene->m_dir_light->draw(light_shader);
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(POSITION));
		/**/glActiveTexture(GL_TEXTURE1);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(DIFFUSE));
		/**/glActiveTexture(GL_TEXTURE2);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(METALLIC));
		/**/glActiveTexture(GL_TEXTURE3);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(NORMAL));
		/**/m_models[2]->m_meshes[0]->draw(light_shader);
		/**/
		/**/// Render Lights
		/**/light_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "render_diffuse_specular");
		/**/scene_cam.set_uniforms(light_shader);
		/**/light_shader->set_uniform("window_width", window_manager->get_width());
		/**/light_shader->set_uniform("window_height", window_manager->get_height());
		/**/GL_CALL(glEnable(GL_BLEND));
		/**/GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/glDepthFunc(GL_GREATER);
		/**/glEnable(GL_CULL_FACE);
		/**/scene->draw_point_lights(light_shader);
		/**/glDisable(GL_CULL_FACE);
		/**/glDepthFunc(GL_LESS);
		/**/skybox_shader->use();
		/**/scene_cam.set_uniforms(skybox_shader);
		/**/skybox_shader->set_uniform("width", (float)light_buffer.m_width);
		/**/skybox_shader->set_uniform("height", (float)light_buffer.m_height);
		/**/skybox_shader->set_uniform("M", glm::translate(mat4(1),scene_cam.m_eye));
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(NORMAL));
		/**/glActiveTexture(GL_TEXTURE1);
		/**/glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.m_id);
		/**/m_models[0]->m_meshes[0]->draw(skybox_shader);
		/**/GL_CALL(glDisable(GL_DEPTH_TEST));
		/**/GL_CALL(glDepthMask(GL_TRUE));
		/**/GL_CALL(glDisable(GL_BLEND));
		///////////////////////////////////////////////////////////////////////////
	}
	if (blur_shader->is_valid())
	{
		// Blur Pass	///////////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, blur_control_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, blur_control_buffer.m_width, blur_control_buffer.m_height));
		/**/blur_shader->use();
		/**/blur_shader->set_uniform("blur_mode", m_render_options.blur_mode);
		/**/blur_shader->set_uniform("bilat_scale", m_render_options.bilat_scale);
		/**/blur_shader->set_uniform("bilat_weight", m_render_options.bilat_weight);
		/**/ortho_cam.set_uniforms(blur_shader);
		/**/GL_CALL(glEnable(GL_BLEND));
		/**/
		/**/// Sobel Edge Detection
		/**/if (m_render_options.do_antialiasing)
		/**/ {
		/**/	blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_sobel_edge_detection");
		/**/	blur_shader->set_uniform("width", (float)blur_control_buffer.m_width);
		/**/	blur_shader->set_uniform("height", (float)blur_control_buffer.m_height);
		/**/	blur_shader->set_uniform("coef_normal", m_render_options.aa_coef_normal);
		/**/	blur_shader->set_uniform("coef_depth", m_render_options.aa_coef_depth);
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(NORMAL));
		/**/	glActiveTexture(GL_TEXTURE1);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(DEPTH));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/}
		/**/
		/**/// Depth of Field
		/**/if (m_render_options.do_depth_of_field)
		/**/ {
		/**/	blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_depth_of_field");
		/**/	blur_shader->set_uniform("focal_distance", m_render_options.df_plane_focus);
		/**/	blur_shader->set_uniform("aperture", m_render_options.df_aperture);
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(POSITION));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/}
		/**/
		/**/// Motion Blur
		/**/if (m_render_options.do_motion_blur)
		/**/ {
		/**/	blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_motion_blur");
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(SELECTION));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/}
		/**/
		/**/// Bloom Filter
		/**/if (m_render_options.do_bloom)
		/**/ {
		/**/	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer.m_fbo));
		/**/	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/	GL_CALL(glViewport(0, 0, bloom_buffer.m_width, bloom_buffer.m_height));
		/**/	// Get Luminance Color
		/**/	bloom_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 });
		/**/	blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_bloom");
		/**/	blur_shader->set_uniform("bloom_coef", m_render_options.bl_coef);
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(LIGHT));
		/**/	glActiveTexture(GL_TEXTURE2);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(LIN_DEPTH));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/	glFlush;
		/**/	glFinish;
		/**/	blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_bloom_blur");
		/**/	blur_shader->set_uniform("width", (float)bloom_buffer.m_width);
		/**/	blur_shader->set_uniform("height", (float)bloom_buffer.m_height);
		/**/	for (int i = 0; i < m_render_options.blur_bloom_iterations; i++)
		/**/	{
		/**/		bloom_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 + 1 });
		/**/		blur_shader->set_uniform("pass", 0);
		/**/		glActiveTexture(GL_TEXTURE0);
		/**/		glBindTexture(GL_TEXTURE_2D, get_texture(BLOOM));
		/**/		m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/		bloom_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 });
		/**/		blur_shader->set_uniform("pass", 1);
		/**/		glActiveTexture(GL_TEXTURE0);
		/**/		glBindTexture(GL_TEXTURE_2D, get_texture(BLOOM2));
		/**/		m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/	}
		/**/	bloom_buffer.set_drawbuffers();	
		/**/}
		/**/else
		/**/{
		/**/	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer.m_fbo));
		/**/	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/}
		/**/
		/**/// Final Blur
		/**/glFlush;
		/**/glFinish;
		/**/GL_CALL(glDisable(GL_BLEND));
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, blur_buffer.m_fbo));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glViewport(0, 0, blur_buffer.m_width, blur_buffer.m_height));
		/**/blur_shader->set_uniform_subroutine(GL_FRAGMENT_SHADER, "do_final_blur");
		/**/blur_shader->set_uniform("width", (float)blur_buffer.m_width);
		/**/blur_shader->set_uniform("height", (float)blur_buffer.m_height);
		/**/glActiveTexture(GL_TEXTURE1);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(BLUR_CONTROL));
		/**/glActiveTexture(GL_TEXTURE2);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(LIN_DEPTH));
		/**/blur_shader->set_uniform("final_mode", 0);
		/**/blur_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 + 1 });
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(LIGHT));
		/**/m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/for (int i = 0; i < m_render_options.blur_general_iterations; i++)
		/**/{
		/**/	blur_shader->set_uniform("final_mode", 1);
		/**/	blur_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 });
		/**/	blur_shader->set_uniform("pass", 0);
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(BLUR2));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/	blur_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0+1 });
		/**/	blur_shader->set_uniform("pass", 1);
		/**/	glActiveTexture(GL_TEXTURE0);
		/**/	glBindTexture(GL_TEXTURE_2D, get_texture(BLUR));
		/**/	m_models[2]->m_meshes[0]->draw(blur_shader);
		/**/}
		/**/blur_shader->set_uniform("final_mode", 2);
		/**/blur_buffer.set_drawbuffers({ GL_COLOR_ATTACHMENT0 });
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(BLUR2));
		/**/glActiveTexture(GL_TEXTURE2);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(BLOOM));
		/**/m_models[2]->m_meshes[0]->draw(blur_shader);
		///////////////////////////////////////////////////////////////////////////
	}



	if (texture_shader->is_valid())
	{
		// Last Render Pass	///////////////////////////////////////////////////////
		/**/GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		/**/GL_CALL(glClearColor(0.50f, 0.75f, 0.93f, 1.0f));
		/**/GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		/**/GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		/**/GL_CALL(glViewport(0, 0, window_manager->get_width(), window_manager->get_height()));
		/**/texture_shader->use();
		/**/ortho_cam.set_uniforms(texture_shader);
		/**/glActiveTexture(GL_TEXTURE0);
		/**/glBindTexture(GL_TEXTURE_2D, get_texture(m_txt_cur));
		/**/GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/m_models[2]->m_meshes[0]->draw(texture_shader);
		/**/GL_CALL(glDisable(GL_DEPTH_TEST));
		/**/if (m_render_options.df_auto_focus)
		/**/{
		/**/	color_shader->use();
		/**/	ortho_cam.set_uniforms(color_shader);
		/**/	color_shader->set_uniform("M", glm::scale(mat4(1.0f), vec3(0.005f)));
		/**/	color_shader->set_uniform("color", vec3(0, 0, 0));
		/**/	GL_CALL(glEnable(GL_DEPTH_TEST));
		/**/	m_models[3]->m_meshes[0]->draw(color_shader);
		/**/	GL_CALL(glDisable(GL_DEPTH_TEST));
		/**/}
		///////////////////////////////////////////////////////////////////////////
	}
}

void c_renderer::shutdown()
{
	delete g_buffer_shader;
	delete decal_shader;
	delete light_shader;
	delete ao_shader;
	delete skybox_shader;
	delete blur_shader;
	delete texture_shader;
	delete color_shader;

	// Clean Meshes
	for (auto m : m_models)
		delete m;
	m_models.clear();
}

void c_renderer::drawGUI()
{
	if (ImGui::TreeNode("Camera"))
	{
		ImGui::SliderFloat("Near", &renderer->scene_cam.m_near, 0.1f, renderer->scene_cam.m_far);
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
					if (ImGui::Selectable((std::to_string(n) + "_" + scene->m_objects[n]->m_model->m_name).c_str(), false))
						scene_cam.use_target(scene->m_objects[n]);
				}
				ImGui::EndCombo();
			}
		}
		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Buffers"))
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
		if (ImGui::TreeNode("G-Buffer"))
		{
			show_image(c_renderer::DIFFUSE);
			show_image(c_renderer::POSITION);
			show_image(c_renderer::METALLIC);
			show_image(c_renderer::NORMAL);
			show_image(c_renderer::LIN_DEPTH);
			show_image(c_renderer::SELECTION);
			show_image(c_renderer::AO);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light"))
		{
			show_image(c_renderer::LIGHT);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Blur"))
		{
			show_image(c_renderer::BLUR_CONTROL);
			show_image(c_renderer::BLOOM);
			show_image(c_renderer::BLOOM2);
			show_image(c_renderer::BLUR);
			show_image(c_renderer::BLUR2);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("RenderOptions"))
	{
		if (ImGui::Button("Recompile Shaders"))
		{
			Shader_Program ** sh[]{ &g_buffer_shader, &decal_shader, &light_shader, &skybox_shader, &ao_shader, &blur_shader, &texture_shader, &color_shader };
			for (Shader_Program ** s : sh)
				*s = new Shader_Program((*s)->paths[0], (*s)->paths[1], (*s)->paths[2], (*s)->paths[3], (*s)->paths[4]);
		}
		ImGui::Checkbox("Render Lights", &m_render_options.render_lights);

		if(ImGui::TreeNode("Curves"))
		{
			ImGui::Checkbox("Render Curves", &m_render_options.render_curves);
			bool updt{ false };
			if (ImGui::SliderFloat("Curve Epsilon", &curve_base::m_epsilon, 0.0001f, 10, "%.4f", 10.f))
				updt = true;
			if (ImGui::InputInt("Forced Subdivision", &curve_base::m_forced_subdivision, 1, 10))
				updt = true, curve_base::m_forced_subdivision = max(0, curve_base::m_forced_subdivision);
			if(updt)
				for (auto& c : m_curves)
					c->do_adaptive_forward_differencing();

			ImGui::NewLine();
			for (auto& c : m_curves)
			{
				if (ImGui::TreeNode(c->m_name.c_str()))
				{
					if (ImGui::TreeNode("Easeing"))
					{
						ImGui::Checkbox("Break Tangents", &curve_base::m_break_tangents);
						c->draw_easing();
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Table"))
					{
						ImGui::Columns(3, "mixed");
						ImGui::Text("Index");
						ImGui::NextColumn();

						ImGui::Text("Parameter");
						ImGui::NextColumn();

						ImGui::Text("Distance");
						ImGui::Columns(1);
						ImGui::Separator();
						int i = 0;
						for (auto& k : c->m_length_table)
						{
							ImGui::Columns(3, "mixed");
							ImGui::Text(std::to_string(i).c_str());
							ImGui::NextColumn();

							ImGui::Text(std::to_string(k.m_param_value).c_str());
							ImGui::NextColumn();

							ImGui::Text(std::to_string(k.m_arclength).c_str());
							ImGui::Columns(1);
							ImGui::Separator();
							i++;
						
						}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("View"))
					{
						if (c->m_target)
						{
							std::string name = c->m_target->m_model
								? c->m_target->m_model->m_name
								: "Unknown";
							ImGui::Text(("View target: " + name).c_str());
							ImGui::SameLine();
							if (ImGui::Button("Release"))
								c->m_target = nullptr;
						}
						else
						{
							if (ImGui::BeginCombo("Target", "Select Obj"))
							{
								for (size_t n = 0; n < scene->m_objects.size(); n++)
								{
									if (ImGui::Selectable((std::to_string(n)+"_"+scene->m_objects[n]->m_model->m_name).c_str(), false))
										c->m_target = scene->m_objects[n];
								}
								ImGui::EndCombo();
							}
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Decals"))
		{
			ImGui::Checkbox("Active", &m_render_options.dc_active);
			ImGui::SliderInt("View Mode", &m_render_options.dc_mode, 0,2);
			ImGui::SliderFloat("Angle Threshold", &m_render_options.dc_angle, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Antialiasing"))
		{
			ImGui::Checkbox("Do Antialiasing", &m_render_options.do_antialiasing);
			ImGui::SliderFloat("Normal Coefficient", &m_render_options.aa_coef_normal, 0.0f, 1.0f);
			ImGui::SliderFloat("Depth Coefficient", &m_render_options.aa_coef_depth, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Depth of Field"))
		{
			ImGui::Checkbox("Do Depth of Field", &m_render_options.do_depth_of_field);
			ImGui::DragFloat("Plane in Focus", &m_render_options.df_plane_focus, 1.0f, 0.0f, 999.f);
			ImGui::DragFloat("Aperture", &m_render_options.df_aperture, 1.0f, 0.0f, 999.f);
			ImGui::Checkbox("Auto Focus", &m_render_options.df_auto_focus);
			if (m_render_options.df_auto_focus)
			{
				glFlush();
				glFinish();
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glBindFramebuffer(GL_FRAMEBUFFER, renderer->g_buffer.m_fbo);

				vec4 data;
				auto wpos = window_manager->m_window->get_mouse_pos();
				glReadPixels(window_manager->get_width()/2, window_manager->get_height()/2, 1, 1, GL_RGBA, GL_FLOAT, &data.x);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				m_render_options.df_plane_focus = -data.z;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Motion Blur"))
		{
			ImGui::Checkbox("Do Motion Blur", &m_render_options.do_motion_blur);
			ImGui::Checkbox("Camera Motion Blur", &m_render_options.mb_camera_blur);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Bloom"))
		{
			ImGui::Checkbox("Do Bloom", &m_render_options.do_bloom);
			ImGui::SliderFloat("Brightness Threshold", &m_render_options.bl_coef, 0.0f, 2.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Ambient Occlusion"))
		{
			ImGui::SliderFloat("Radius", &m_render_options.ao_radius, 0.01f, 20.0f);
			ImGui::SliderFloat("Angle Bias", &m_render_options.ao_angle_bias, 0.00f, glm::half_pi<float>());
			ImGui::SliderInt("Num Dirs", &m_render_options.ao_num_dirs, 1, 20);
			ImGui::SliderInt("Num Steps", &m_render_options.ao_num_steps, 1, 20);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Blur"))
		{
			ImGui::SliderInt("Blur Bloom Iterations", &m_render_options.blur_bloom_iterations, 0, 10);
			ImGui::SliderInt("Blur General Iterations", &m_render_options.blur_general_iterations, 0, 10);
			static const char* modes[] = { "7x7 Gaussian", "7x7 Gaussian Bilateral" };
			if (ImGui::BeginCombo("Mode", modes[m_render_options.blur_mode]))
			{
				for (int n = 0; n < (int)IM_ARRAYSIZE(modes); n++)
				{
					if (ImGui::Selectable(modes[n], n == m_render_options.blur_mode))
						m_render_options.blur_mode = n;
				}
				ImGui::EndCombo();
			}
			ImGui::DragFloat("Bilateral Threshold", &m_render_options.bilat_scale, 10.0f, 1.0f, 1000.0f);
			ImGui::SliderFloat("Bilateral Weight", &m_render_options.bilat_weight,0.0f, 1.0f);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}

void c_renderer::set_debug_color(vec3 c)
{
	Model::m_def_materials[0]->m_albedo = c;
}

void c_renderer::reset_debug_color()
{
	Model::m_def_materials[0]->m_albedo = { 0,1,0 };
}

GLuint c_renderer::get_texture(e_texture ref)
{
	switch (ref)
	{
	case c_renderer::e_texture::POSITION:
		return g_buffer.m_color_texture[0]; 
	case c_renderer::e_texture::DIFFUSE:
		return g_buffer.m_color_texture[1];
	case c_renderer::e_texture::METALLIC:
		return g_buffer.m_color_texture[2];
	case c_renderer::e_texture::NORMAL:
		return g_buffer.m_color_texture[3];
	case c_renderer::e_texture::LIN_DEPTH:
		return g_buffer.m_color_texture[4];
	case c_renderer::e_texture::DEPTH:
		return g_buffer.m_depth_texture;
	case c_renderer::e_texture::SELECTION:
		return selection_buffer.m_color_texture[0];
	case c_renderer::e_texture::AO:
		return ao_buffer.m_color_texture[0];
	case c_renderer::e_texture::LIGHT:
		return light_buffer.m_color_texture[0];
	case c_renderer::e_texture::BLUR_CONTROL:
		return blur_control_buffer.m_color_texture[0];
	case c_renderer::e_texture::BLOOM:
		return bloom_buffer.m_color_texture[0];
	case c_renderer::e_texture::BLOOM2:
		return bloom_buffer.m_color_texture[1];
	case c_renderer::e_texture::BLUR:
		return blur_buffer.m_color_texture[0];
	case c_renderer::e_texture::BLUR2:
		return blur_buffer.m_color_texture[1];
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
