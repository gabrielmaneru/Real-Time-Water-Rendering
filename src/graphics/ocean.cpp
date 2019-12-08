#include "ocean.h"
#include "gl_error.h"
#include "renderer.h"
#include <platform/window.h>
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <utils/generate_noise.h>
#include <stb_image/stb_image.h>
#include <iostream>
noise_layer::noise_layer(size_t resolution, float noise_scale, int iterations, float complexity, size_t layer_count, float speed, float height, vec2 dir)
	: m_noise_scale(noise_scale), m_iterations(iterations), m_complexity(complexity),
	m_layer_count(layer_count), m_speed(speed), m_height(height), m_mode(0u), m_dirs(straight(dir, resolution)), m_direction(dir), m_time(0.0f)
{
	build_layers(resolution);
}

noise_layer::noise_layer(size_t resolution, float noise_scale, int iterations, float complexity, size_t layer_count, float speed, float height)
	: m_noise_scale(noise_scale), m_iterations(iterations), m_complexity(complexity),
	m_layer_count(layer_count), m_speed(speed), m_height(height), m_mode(1u), m_dirs(whirlpool(resolution)), m_time(0.0f)
{
	build_layers(resolution);
}

void noise_layer::build_layers(size_t resolution)
{
	m_noise_layers.clear();
	for (size_t i = 0; i < m_layer_count; i++)
	{
		randomize_noise();
		m_noise_layers.emplace_back(generate_noise(resolution, ((float)resolution)*m_noise_scale, m_iterations, m_complexity, 1.0f / m_complexity));
	}
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
			if(height_scale > 0.0f)
				dir = glm::normalize(dir);

			float value = 0.0f;
			for(size_t i = 0; i < m_layer_count; i++)
			{
				if (w[i] > 0.0f)
				{
					vec2 uv_temp = clamp(uv + dir * m_uv_step * f[i], 0.0f, 1.0f);
					float val = m_noise_layers[i].get_linear(uv_temp*vec2((float)scale - 1));
					value += val * w[i];
				}
			}
			vertices[y*scale + x].y += m_height * value * height_scale;
		}
}

void Ocean::init()
{
	m_noise.emplace_back(new noise_layer{ m_resolution, 0.1f, 2, 2.0f,
		3u, 0.2f, 5.0f, {0.0f, 1.0f} });
	m_noise.emplace_back(new noise_layer{ m_resolution, 2.0f, 4, 4.0f,
		3u, 0.3f, 2.0f, {0.0f, 1.0f} });
	m_noise.emplace_back(new noise_layer{ m_resolution, 8.0f, 4, 4.0f,
		3u, 0.5f, 0.5f, {0.0f, 1.0f} });
	m_noise.emplace_back(new noise_layer{ m_resolution, 0.05f, 2, 2.0f,
		3u, 0.2f, 7.5f, {0.0f, -1.0f} });

	m_mesh.build_plane((int)m_resolution, m_mesh_scale);
	m_mesh.compute_normals();
	m_mesh.load();

	m_caustics.setup(256, 256);
	m_caustics.clear(0.0f);
}

void Ocean::draw(Shader_Program* shader)
{
	//GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	//shader->set_uniform("line_render", true);
	//m_mesh.draw();

	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	shader->set_uniform("line_render", false);
	m_mesh.draw();
}

void Ocean::drawGUI()
{
	bool reset_all{ false };
	if (ImGui::InputUInt("Resolution", &m_resolution)) reset_all = true;
	if (ImGui::InputFloat("Mesh Scale", &m_mesh_scale)) reset_all = true;
	if(reset_all)
		m_mesh.build_plane((int)m_resolution, m_mesh_scale);

	if (ImGui::Button("Add New Layer"))
		m_noise.emplace_back(new noise_layer{ m_resolution, 5.0f, 4, 4.0f,
			3u, 0.5f, 1.0f, {0.0f, 1.0f} });
	for (size_t l = 0; l < m_noise.size(); l++)
	{
		std::string name = "Layer " + std::to_string(l);
		noise_layer* layer = m_noise[l];
		if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::Text("Noise Properties");
			bool reset_noise{ false };

			float n_scale = layer->m_noise_scale;
			if (ImGui::InputFloat("Noise Scale", &n_scale)) reset_noise = true;

			int it = layer->m_iterations;
			if (ImGui::InputInt("Iterations", &it))reset_noise = true;

			float complex = layer->m_complexity;
			if (ImGui::InputFloat("Iteration Complexity", &complex)) reset_noise = true;

			size_t l_count = layer->m_layer_count;
			if (ImGui::InputUInt("Interpolation Layer Count", &l_count)) reset_noise = true;

			if (reset_noise || reset_all)
			{
				noise_layer * next{nullptr};
				if (layer->m_mode == 0)
					next = new noise_layer{ m_resolution, n_scale, it, complex,
						l_count, layer->m_speed, layer->m_height, layer->m_dirs.get(0)};
				else if(layer->m_mode == 1)
					next = new noise_layer{ m_resolution, n_scale, it, complex,
						l_count, layer->m_speed, layer->m_height };
				delete layer;
				layer = m_noise[l] = next;
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::SliderFloat("Speed", &layer->m_speed, 0.f, 5.f);
			ImGui::SliderFloat("Height", &layer->m_height, 0.f, 10.f);

			const char * modes[] = { "Straight", "Whirpool" };
			if (ImGui::BeginCombo("Direction Modes", modes[layer->m_mode]))
			{
				for (size_t n = 0; n < 2; n++)
				{
					if (ImGui::Selectable(modes[n], layer->m_mode == n))
					{
						layer->m_mode = n;
						if (layer->m_mode == 0)
							layer->m_dirs = straight(layer->m_direction, m_resolution);
						if (layer->m_mode == 1)
							layer->m_dirs = whirlpool(m_resolution);
					}
				}
				ImGui::EndCombo();
			}
			if (layer->m_mode == 0)
				if(ImGui::SliderFloat2("Direction", &layer->m_direction.x, -1.0f, 1.0f))
					layer->m_dirs = straight(layer->m_direction, m_resolution);

			if (ImGui::Button("Remove Layer"))
			{
				m_noise.erase(m_noise.begin() + l);
				ImGui::TreePop();
				return;
			}
			ImGui::TreePop();
		}
		else if (reset_all)
		{
			noise_layer * next{ nullptr };
			if (layer->m_mode == 0)
				next = new noise_layer{ m_resolution, layer->m_noise_scale, layer->m_iterations, layer->m_complexity,
					layer->m_layer_count, layer->m_speed, layer->m_height, layer->m_dirs.get(0) };
			else if (layer->m_mode == 1)
				next = new noise_layer{ m_resolution, layer->m_noise_scale, layer->m_iterations, layer->m_complexity,
					layer->m_layer_count, layer->m_speed, layer->m_height };
			delete layer;
			layer = m_noise[l] = next;
		}

	}
}

void Ocean::update_mesh()
{
	for (auto& v : m_mesh.vertices)
		v.y = 0.0f;

	float dt = (float)(1.0 / window::frameTime);

	for (auto& l : m_noise)
	{
		l->apply_height(m_mesh.vertices);
		l->update_time(dt);
	}

	m_mesh.compute_normals();
	m_mesh.load();

	map2d<vec3> copy_norm{ m_resolution, m_resolution };
	copy_norm.loop([&](size_t x, size_t y, vec3)->vec3
	{
		return m_mesh.normals[y*m_resolution + x];
	});
	m_caustics.loop([&](size_t x, size_t y, float)->float
	{
		return glm::normalize(copy_norm.get_linear(vec2{
			map<size_t,float>(x, 0u, m_caustics.m_width - 1, 0.0f, (float)m_resolution-1),
			map<size_t,float>(y, 0u, m_caustics.m_height - 1, 0.0f, (float)m_resolution-1)
			})).y;
	});
	m_caustics.load();
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