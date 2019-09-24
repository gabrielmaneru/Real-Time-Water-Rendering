#pragma once
#include "shader_program.h"
#include "vectorial_camera.h"
#include "ortho_camera.h"
#include "model.h"
#include "framebuffer.h"

class c_renderer
{
	// Shaders
	Shader_Program* color_shader;
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
		DIFFUSE_rgb,
		POSITION,
		POSITION_rgb,
		NORMAL,
		NORMAL_rgb,
		DEPTH,
		LIGHT
	}m_txt_cur{ NORMAL_rgb };

public:
	bool init();
	void update();
	void shutdown();
	void drawGUI();

	GLuint get_texture(e_texture ref);
	void set_texture(e_texture ref) { m_txt_cur = ref; }
	const Model* get_model(std::string s);

	friend class c_editor;
	friend class light;
};
extern c_renderer* renderer;