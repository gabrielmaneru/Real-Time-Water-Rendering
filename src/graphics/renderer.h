#pragma once
#include "shader_program.h"
#include "vectorial_camera.h"
#include "ortho_camera.h"
#include "mesh.h"
class c_renderer
{
	// Shaders
	Shader_Program* color_shader;
	Shader_Program* texture_shader;

	// Cameras
	vectorial_camera scene_cam{};
	ortho_camera ortho_cam{};

	// Meshes
	std::vector<Mesh*> m_meshes;
	enum e_meshes{ cube=0, octohedron, quad, segment, sphere };
	

public:
	bool init();
	void update();
	void shutdown();
	friend class c_editor;
	friend struct generator;
	friend class eroder;
	friend class MeshReference;
};
extern c_renderer* renderer;