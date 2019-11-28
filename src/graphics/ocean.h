#pragma once
#include "raw_texture.h"
#include "raw_mesh.h"
#include "shader_program.h"


struct noise_layer
{
	noise_layer(size_t resolution, float noise_scale, int iterations, float complexity,	size_t layer_count,
		float speed, float height, vec2 dir);
	noise_layer(size_t resolution, float noise_scale, int iterations, float complexity, size_t layer_count,
		float speed, float height);
	void build_layers();
	void update_time(float dt);
	void apply_height(std::vector<vec3>& vertices);

	const size_t m_resolution;
	const float m_noise_scale;
	const int m_iterations;
	const float m_complexity;
	const size_t m_layer_count;

	float m_speed;
	float m_height;
	size_t m_mode;
	map2d<vec2> m_dirs;
	vec2 m_direction{ 0.0f, 1.0f };

	float m_time;
	std::vector<map2d<float>> m_noise_layers;
};

struct Ocean
{
	void init();
	void draw(Shader_Program*);
	void drawGUI();
	void update_mesh();

	std::vector<noise_layer*> m_noise;
	raw_mesh m_mesh;
};
map2d<vec2> straight(vec2 dir, size_t scale);
map2d<vec2> whirlpool(size_t scale);
