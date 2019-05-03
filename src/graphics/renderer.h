#pragma once
#include "raw_texture.h"
#include "shader_program.h"
#include "ortho_camera.h"
#include "vectorial_camera.h"
#include "transform2d.h"
#include "framebuffer.h"
class c_renderer
{
	Shader_Program* ray_marching_shader;
	ortho_camera cam;
	vectorial_camera scene_cam;
	transform2d model_texture;
	float blendfactor{ 1.0f };
	int highlightfactor{ 100 };
	struct
	{
		uint32_t vao{ 0 };
		uint32_t cnt{};
	} quad{};

public:
	bool init();
	void update();
	void shutdown();
	friend class c_editor;
};
extern c_renderer* renderer;