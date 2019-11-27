#pragma once
#include "raw_texture.h"
#include "raw_mesh.h"
#include "shader_program.h"

struct Ocean
{
	void init();
	void draw(Shader_Program*);
	void update_mesh();

	float m_time=0.0;
	float uv_step = 0.1f;
	float m_height = 1.0f;
	map2d<float> noise_16[2];
	raw_mesh m_mesh;
};