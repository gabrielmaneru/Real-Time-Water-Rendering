/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	renderer.h
Purpose: OpenGl renderer
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "shader_program.h"
#include "vectorial_camera.h"
#include "ortho_camera.h"
#include "model.h"
#include "framebuffer.h"
#include <glm/glm.h>

class c_renderer
{
	// Shaders
	Shader_Program* g_buffer_shader;
	Shader_Program* light_shader;
	Shader_Program* texture_shader;

	// Cameras
	vectorial_camera scene_cam{};

	// Meshes
	std::vector<Model*> m_models;

	// Framebuffer
	framebuffer g_buffer;
	framebuffer light_buffer;

	enum e_texture {
		DIFFUSE,
		POSITION,
		NORMAL,
		SELECTION,
		DEPTH,
		LIGHT
	}m_txt_cur{ LIGHT };
	
	struct Options
	{
		bool render_lights{false};
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