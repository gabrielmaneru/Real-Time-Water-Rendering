#include "ocean.h"
#include "gl_error.h"
#include <platform/window.h>
#include <GL/gl3w.h>
#include <utils/generate_noise.h>
#include <iostream>


noise_layer::noise_layer(size_t resolution, float noise_scale, int iterations, float complexity,
	size_t layer_count, float speed, map2d<vec2> dirs, float height)
	: m_resolution(resolution), m_noise_scale(noise_scale), m_iterations(iterations), m_complexity(complexity),
	m_layer_count(layer_count), m_speed(speed), m_dirs(dirs), m_height(height), m_time(0.0f)
{
	build_layers();
}

void noise_layer::build_layers()
{
	m_noise_layers.clear();
	randomize_noise();
	for (size_t i = 0; i < m_layer_count; i++)
		m_noise_layers.emplace_back(generate_noise(m_resolution, ((float)m_resolution)*m_noise_scale, m_iterations, m_complexity, 1.0f / m_complexity));
}

void noise_layer::update_time(float dt)
{
	m_time += dt*m_speed;
	if (m_time >= m_layer_count)
		m_time -= m_layer_count;
	else if (m_time < 0.0f)
		m_time += m_layer_count;
}

void noise_layer::apply_height(std::vector<vec3>& vertices)
{
	std::vector<float> w; w.resize(m_layer_count);
	std::vector<float> f; f.resize(m_layer_count);

	float size_f = static_cast<float>(m_layer_count);
	for (size_t i = 0; i < m_layer_count; i++)
	{
		float idx_f = static_cast<float>(i);
		w[i] = max(1.0f - (fabsf(fmod(m_time + size_f*0.5f - idx_f, size_f) - size_f * 0.5f)),0.0f);
		if (w[i] > 0.0f)
			f[i] = fmod(m_time + 1.0f - idx_f, size_f) - 1.0f;
	}

	const float m_uv_step = 0.1f;
	size_t scale = static_cast<size_t>(sqrt(vertices.size()));
	for (size_t y = 0; y < scale; y++)
		for (size_t x = 0; x < scale; x++)
		{
			vec2 uv = vec2{ coef<size_t>(0, scale - 1, x),coef<size_t>(0, scale - 1, y) };
			vec2 dir = m_dirs.get(x, y);
			float height_scale = glm::length(dir);
			height_scale = 1 - glm::pow(1 - height_scale, 4);
			dir = glm::normalize(dir);

			float value = 0.0f;
			for(size_t i = 0; i < m_layer_count; i++)
			{
				if (w[i] > 0.0f)
				{
					vec2 uv_temp = clamp(uv + dir * m_uv_step * f[i], 0.0f, 1.0f);
					float val = m_noise_layers[i].get_linear(uv_temp*vec2(scale - 1));
					value += val * w[i];
				}
			}
			vertices[y*scale + x].y += m_height * value * height_scale;
		}
}

void Ocean::init()
{
	size_t res = 256u;
	m_noise.emplace_back(noise_layer{ res, 0.1f, 4, 2.0f,
		3u, 2.0f, whirlpool(res), 2.0f });

	m_mesh.build_plane(m_noise[0].m_resolution, 0.1f);
	m_mesh.compute_normals();
	m_mesh.load();
}

void Ocean::draw(Shader_Program* shader)
{
	update_mesh();

	//GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	//shader->set_uniform("line_render", true);
	//m_mesh.draw();

	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	shader->set_uniform("line_render", false);
	m_mesh.draw();
}

void Ocean::update_mesh()
{
	for (auto& v : m_mesh.vertices)
		v.y = 0.0f;

	float dt = (float)1.0 / window::frameTime;

	for (auto& l : m_noise)
	{
		l.apply_height(m_mesh.vertices);
		l.update_time(dt);
	}

	m_mesh.compute_normals();
	m_mesh.load();
}

map2d<vec2> straight(vec2 dir, size_t scale)
{
	return map2d<vec2>{scale, scale, dir};
}

map2d<vec2> whirlpool(size_t scale)
{
	map2d<vec2> m{scale,scale};

	m.loop([scale](size_t x, size_t y, vec2)->vec2
	{
		float x_ = map<size_t,float>(x, 0, scale - 1, -1.0f, 1.0f);
		float y_ = map<size_t, float>(y, 0, scale - 1, -1.0f, 1.0f);

		vec2 pos = { x_,y_ };
		float r = glm::length(pos);
		float theta = atan2f(y_, x_);
		float extra_angle = map(r, 0.0f, glm::sqrt(2.0f), glm::half_pi<float>(), 0.0f);
		float new_theta = theta + extra_angle;
		vec2 new_pos = { cosf(new_theta),sinf(new_theta) };

		return glm::normalize(new_pos - pos)*coef(0.0f, glm::sqrt(2.0f), r);
	});
	return m;
}
