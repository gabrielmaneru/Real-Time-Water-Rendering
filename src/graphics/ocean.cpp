#include "ocean.h"
#include "gl_error.h"
#include <platform/window.h>
#include <GL/gl3w.h>
#include <utils/generate_noise.h>
#include <iostream>

void Ocean::init()
{

	randomize_noise();
	int noise_size = 64u;
	noise_16[0] = generate_noise(noise_size, 16.0f, 4, 2.0f, 0.5f);
	noise_16[1] = generate_noise(noise_size, 16.0f, 4, 2.0f, 0.5f);

	m_mesh.build_from_map(noise_16[0], 0.1f, m_height);
	m_mesh.compute_normals();
	m_mesh.load();
}

void Ocean::draw(Shader_Program* shader)
{
	update_mesh();

	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	shader->set_uniform("line_render", true);
	m_mesh.draw();

	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	shader->set_uniform("line_render", false);
	m_mesh.draw();
}

void Ocean::update_mesh()
{
	float a_f = fmod(m_time + 1.0f, 2.0f) - 1.0f;
	float b_f = fmod(m_time, 2.0f) - 1.0f;
	float a_w = fabsf(m_time - 1.0);
	float b_w = 1. - a_w;

	size_t scale = static_cast<size_t>(sqrt(m_mesh.vertices.size()));
	for (size_t y = 0; y < scale; y++)
		for (size_t x = 0; x < scale; x++)
		{
			vec2 uv = vec2{ coef<size_t>(0, scale-1, x),coef<size_t>(0, scale-1, y) };
			const vec2 dir = { 0,1 };
			float value = 0.0f;
			{
				vec2 uv_a = clamp(uv + dir * uv_step * a_f, 0.0f, 1.0f);
				float val = noise_16[0].get_linear(uv_a*vec2(scale-1));
				value += val * a_w;
			}
			{
				vec2 uv_b = clamp(uv + dir * uv_step * b_f, 0.0f, 1.0f);
				float val = noise_16[1].get_linear(uv_b*vec2(scale-1));
				value += val * b_w;
			}
			m_mesh.vertices[y*scale + x].y = m_height * value;
		}
	m_mesh.compute_normals();
	m_mesh.load();
	m_time += (float)1.0/window::frameTime;
	if (m_time >= 2.0f)
		m_time -= 2.0f;
}
