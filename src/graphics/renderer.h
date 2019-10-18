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
	// Shaders
	Shader_Program* g_buffer_shader;
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
	curve_line*    m_curve_line;
	curve_hermite* m_curve_hermite;
	curve_catmull* m_curve_catmull;
	curve_bezier*  m_curve_bezier;

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
		NORMAL,
		SELECTION,
		DEPTH,
		LIGHT,
		BLUR_CONTROL,
		BLOOM,
		BLUR_RESULT
	}m_txt_cur{ BLUR_RESULT };
	
	struct Options
	{
		bool render_lights{ false };
		bool render_curves{ false };

		bool do_antialiasing{ true };
		float aa_coef_normal{ 0.05f };
		float aa_coef_depth{ 0.25f };
		int aa_sigma{ 5 };

		bool do_depth_of_field{ false };
		float df_plane_focus{ 45.f };
		float df_aperture{ 45.f };
		bool df_auto_focus{ false };

		bool do_motion_blur{ true };
		bool mb_camera_blur{ true };

		bool do_bloom{ true };
		float bl_coef{ 1.f };
	}m_render_options;

	std::pair<size_t,size_t> m_selection_calls{0u,0u};
	void update_max_draw_call_count();

public:
	bool init();
	void update();
	void shutdown();
	void drawGUI();

	GLuint get_texture(e_texture ref);
	void set_texture(e_texture ref) { m_txt_cur = ref; }
	const Model* get_model(std::string s);
	vec3 compute_selection_color();

	friend class c_editor;
	friend class c_scene;
	friend class light;
};
extern c_renderer* renderer;