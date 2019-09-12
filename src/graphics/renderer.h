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
	ortho_camera ortho_cam{};

	// Meshes
	std::vector<Model*> m_models;

	// Framebuffer
	framebuffer g_buffer;
	framebuffer light_buffer;

public:
	bool init();
	void update();
	void shutdown();

	const Model* get_model(std::string s);

	friend class c_editor;
	friend struct generator;
	friend class eroder;
	friend struct MeshReference;
};
extern c_renderer* renderer;