#pragma once
#include "raw_texture.h"
#include "shader_program.h"
#include "ortho_camera.h"
#include "transform2d.h"
class c_renderer
{
	raw_texture render_texture;
	Shader_Program* shader;
	ortho_camera cam;
	transform2d model_texture;
	struct
	{
		uint32_t vao{ 0 };
		uint32_t cnt;
	} quad;
	
public:
	bool init();
	void update();
	void shutdown();
};
extern c_renderer* renderer;