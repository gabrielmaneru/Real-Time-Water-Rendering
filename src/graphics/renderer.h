#pragma once
#include "shader_program.h"
#include "vectorial_camera.h"
#include "ortho_camera.h"
#include "model.h"

class c_renderer
{
	// Shaders
	Shader_Program* color_shader;
	Shader_Program* texture_shader;

	// Cameras
	vectorial_camera scene_cam{};

	// Meshes
	std::vector<Model*> m_models;
	

public:
	bool init();
	void update();
	void shutdown();

	enum e_meshes{ cube=0, octohedron, quad, sphere, sponza };
	const Model* get_model(size_t i) { return m_models[i]; }

	friend class c_editor;
	friend struct generator;
	friend class eroder;
	friend struct MeshReference;
};
extern c_renderer* renderer;