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

	float la = 0.0f;
	vec3 ld = vec3{ 1.0};
	vec3 ls = vec3{ 1.0};
	vec3 att_factor = vec3{ 0.01f, 0.01f, 0.01f };

public:
	bool init();
	void update();
	void shutdown();
	void drawGUI();

	GLuint get_texture(e_texture ref);
	void set_texture(e_texture ref) { m_txt_cur = ref; }
	const Model* get_model(std::string s);

	friend class c_editor;
	friend struct generator;
	friend class eroder;
	friend struct MeshReference;
};
extern c_renderer* renderer;