#pragma once
#include "raw_texture.h"
#include "texture.h"
#include "raw_mesh.h"
#include "shader_program.h"


struct noise_layer
{
	noise_layer(size_t resolution, float noise_scale, int iterations, float complexity,	size_t layer_count,
		float speed, float height, vec2 dir);
	noise_layer(size_t resolution, float noise_scale, int iterations, float complexity, size_t layer_count,
		float speed, float height);
	void build_layers(size_t resolution);
	void update_time(float dt);
	void apply_height(std::vector<vec3>& vertices);

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

	size_t m_resolution{ 256u };
	float m_mesh_scale{ 1.f };
	std::vector<noise_layer*> m_noise;
	raw_mesh m_mesh;
	raw_texture_single m_caustics;

	struct Shading
	{
		bool m_wireframe_mode{false};

		float m_shore_distance{ 30.0f };
		float m_shore_color_power{ 2.0f };
		vec3 m_shore_water_color{ 0.00f, 0.47f, 0.37f };
		vec3 m_deep_water_color{ 0.02f, 0.25f, 0.45f };
		float m_shore_blend_power{ 1.2f };

		float m_reflection_step{ 0.01f };
		int m_reflection_step_max{ 500 };
		int m_reflection_refinement_count{ 0 };

		float m_refraction_angle{ 0.9f };

		float m_caustic_power{ 20.0f };
		vec2 m_caustic_interval{ 0.0f, 1.5f };

		vec2 m_light_interval{ 0.0f, 0.75f };
		float m_light_specular{ 1.0f };
	} shade_info{};
};

map2d<vec2> straight(vec2 dir, size_t scale);
map2d<vec2> whirlpool(size_t scale);
