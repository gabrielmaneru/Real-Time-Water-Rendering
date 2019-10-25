/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	renderer.h
Purpose: OpenGl renderer
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "shader_program.h"
#include "vectorial_camera.h"
#include "ortho_camera.h"
#include "model.h"
#include "framebuffer.h"
#include "curve.h"
#include <glm/glm.h>

class c_renderer
{
public:
	// Shaders
	Shader_Program* g_buffer_shader;
	Shader_Program* tesselation_shader;
	Shader_Program* light_shader;
	Shader_Program* blur_shader;
	Shader_Program* texture_shader;
	Shader_Program* color_shader;

	// Cameras
	vectorial_camera scene_cam{};
	ortho_camera ortho_cam{};

	// Meshes
	std::vector<Model*> m_models;

	//Curves
	std::vector<curve_base*> m_curves;

	// Framebuffer
	framebuffer g_buffer;
	framebuffer selection_buffer;
	framebuffer light_buffer;
	framebuffer blur_control_buffer;
	framebuffer bloom_buffer;
	framebuffer blur_buffer;

	enum e_texture {
		DIFFUSE,
		POSITION,
		METALLIC,
		NORMAL,
		ADAPTIVE,
		SELECTION,
		DEPTH,
		LIGHT,
		BLUR_CONTROL,
		BLOOM,
		BLUR_RESULT
	}m_txt_cur{ BLUR_RESULT };
	

	std::pair<size_t,size_t> m_selection_calls{0u,0u};
	void update_max_draw_call_count();

	bool init();
	void update();
	void shutdown();
	void drawGUI();

	GLuint get_texture(e_texture ref);
	void set_texture(e_texture ref) { m_txt_cur = ref; }
	const Model* get_model(std::string s);
	vec3 compute_selection_color();

	struct Options
	{
		bool render_lights{ false };
		bool render_curves{ true };

		bool  do_antialiasing{ true };
		float aa_coef_normal{ 0.05f };
		float aa_coef_depth{ 0.25f };
		int   aa_sigma{ 5 };

		bool  do_depth_of_field{ false };
		float df_plane_focus{ 45.f };
		float df_aperture{ 45.f };
		bool  df_auto_focus{ false };

		bool  do_motion_blur{ false };
		bool  mb_camera_blur{ false };

		bool  do_bloom{ true };
		float bl_coef{ 1.f };

		float tess_levels{ 3.0f };
		float tess_alpha{ 1.0f };
		bool  tess_useadaptive{ true };
		bool  tess_wireframe{ false };
		bool tess_uselod{ true };
		float tess_lod{ 5.0f };
		float tess_lodpower{ 50.0f };
	}m_render_options;

	friend class c_editor;
	friend class scene_object;
	friend struct curve_interpolator;
	friend class c_scene;
	friend class light;
};
extern c_renderer* renderer;