#include "ocean.h"
#include "gl_error.h"
#include "renderer.h"
#include <platform/window.h>
#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <utils/generate_noise.h>
#include <stb_image/stb_image.h>
#include <scene/scene.h>
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

	m_mesh.build_plane((int)m_resolution, m_mesh_scale);
	m_mesh.compute_normals();
	m_mesh.load();

	m_caustics.setup(256, 256);
	m_caustics.clear(0.0f);
	m_foam.loadFromFile(Texture::filter_name("foam.jpg").c_str());
}

void Ocean::draw(Shader_Program* shader)
{
	if (shade_info.m_wireframe_mode)
		GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE))
	else
		GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))

	shader->set_uniform("WireframeMode", shade_info.m_wireframe_mode);
	shader->set_uniform("ShoreDistance", shade_info.m_shore_distance);
	shader->set_uniform("ShoreColorPower", shade_info.m_shore_color_power);
	shader->set_uniform("ShoreWaterColor", shade_info.m_shore_water_color);
	shader->set_uniform("DeepWaterColor", shade_info.m_deep_water_color);
	shader->set_uniform("ShoreBlendPower", shade_info.m_shore_blend_power);

	shader->set_uniform("DoReflection", shade_info.m_do_reflection);
	shader->set_uniform("ReflectionStep", shade_info.m_reflection_step);
	shader->set_uniform("ReflectionStepMax", shade_info.m_reflection_step_max);
	shader->set_uniform("ReflectionRefinementCount", shade_info.m_reflection_refinement_count);
	shader->set_uniform("ReflectionMaxPen", shade_info.m_reflection_maxpen);

	shader->set_uniform("DoRefraction", shade_info.m_do_refraction);
	shader->set_uniform("RefractionAngle", shade_info.m_refraction_angle);

	shader->set_uniform("DoCaustic", shade_info.m_do_caustic);
	shader->set_uniform("CausticPower", shade_info.m_caustic_power);
	shader->set_uniform("CausticInterval", shade_info.m_caustic_interval);

	shader->set_uniform("DoLighting", shade_info.m_do_lighting);
	shader->set_uniform("LightInterval", shade_info.m_light_interval);
	shader->set_uniform("LightSpecular", shade_info.m_light_specular);

	shader->set_uniform("DoFoam", shade_info.m_do_foam);


	m_mesh.draw();
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))
}

void Ocean::drawGUI()
{
	if (ImGui::TreeNode("Layers:"))
	{
		bool reset_all{ false };
		if (ImGui::InputUInt("Resolution", &m_resolution)) reset_all = true;
		if (ImGui::InputFloat("Mesh Scale", &m_mesh_scale)) reset_all = true;
		if(reset_all)
			m_mesh.build_plane((int)m_resolution, m_mesh_scale);

		if (ImGui::Button("Add New Layer"))
			m_noise.emplace_back(new noise_layer{ m_resolution, 5.0f, 4, 4.0f,
				3u, 0.5f, 1.0f, {0.0f, 1.0f} });
		if (ImGui::Button("Reset Default"))
		{
			m_resolution = 256;
			m_mesh.build_plane((int)m_resolution, m_mesh_scale);
			m_noise.clear();
			m_noise.emplace_back(new noise_layer{ m_resolution, 0.1f, 2, 2.0f,
				3u, 0.2f, 5.0f, {0.0f, 1.0f} });
			m_noise.emplace_back(new noise_layer{ m_resolution, 2.0f, 4, 4.0f,
				3u, 0.3f, 2.0f, {0.0f, 1.0f} });
			m_noise.emplace_back(new noise_layer{ m_resolution, 8.0f, 4, 4.0f,
				3u, 0.5f, 0.5f, {0.0f, 1.0f} });
			m_noise.emplace_back(new noise_layer{ m_resolution, 0.05f, 2, 2.0f,
				3u, 0.2f, 7.5f, {0.0f, -1.0f} });
		}

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
				ImGui::SliderFloat("Height", &layer->m_height, 0.f, 25.f);

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
				{
					float ang = atan2(layer->m_direction.y, layer->m_direction.x);
					if (ImGui::SliderAngle("Dir", &ang, -180.f, 180.f))
					{
						layer->m_direction = { cosf(ang),sinf(ang) };
						layer->m_dirs = straight(layer->m_direction, m_resolution);
					}
				}

				if (ImGui::Button("Remove Layer"))
				{
					m_noise.erase(m_noise.begin() + l);
					ImGui::TreePop();
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
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Shading:"))
	{
		ImGui::Checkbox("Wireframe", &shade_info.m_wireframe_mode);
		ImGui::Checkbox("Render Terrain", &shade_info.m_render_terrain);
		ImGui::Checkbox("Render Props", &shade_info.m_render_props);

		ImGui::Text("Shore Properties"); ImGui::SameLine(); if (ImGui::Button("R"))shade_info.m_shore_distance = { 30.0f }, shade_info.m_shore_color_power = { 2.0f }, shade_info.m_shore_water_color = { 0.00f, 0.47f, 0.37f }, shade_info.m_deep_water_color = { 0.02f, 0.25f, 0.45f }, shade_info.m_shore_blend_power = { 1.2f };
		ImGui::SliderFloat("Distance", &shade_info.m_shore_distance, 0.1f, 100.0f);
		ImGui::SliderFloat("Color Power", &shade_info.m_shore_color_power, 0.1f, 10.0f);
		ImGui::ColorEdit3("Shore Color", &shade_info.m_shore_water_color.x);
		ImGui::ColorEdit3("Deep Color", &shade_info.m_deep_water_color.x);
		ImGui::SliderFloat("Blend Power", &shade_info.m_shore_blend_power, 0.1f, 10.0f);

		ImGui::Checkbox("Reflection", &shade_info.m_do_reflection);
		if (shade_info.m_do_reflection)
		{
			ImGui::PushID(1);
			ImGui::SameLine(); if (ImGui::Button("R"))shade_info.m_reflection_step = { 0.01f }, shade_info.m_reflection_step_max = { 500 }, shade_info.m_reflection_refinement_count = { 0 }, shade_info.m_reflection_maxpen = {0.1f};
			ImGui::SliderFloat("Step", &shade_info.m_reflection_step, 0.01f, 10.0f);
			ImGui::SliderInt("Max Step", &shade_info.m_reflection_step_max, 1, 500);
			ImGui::SliderInt("Refinement", &shade_info.m_reflection_refinement_count, 0, 16);
			ImGui::SliderFloat("Max Penetration", &shade_info.m_reflection_maxpen, 0.0f, 50.0f);
			ImGui::PopID();
		}

		ImGui::Checkbox("Refraction", &shade_info.m_do_refraction);
		if (shade_info.m_do_refraction)
		{
			ImGui::PushID(2);
			ImGui::SameLine(); if (ImGui::Button("R"))shade_info.m_refraction_angle = { 0.9f };
			ImGui::SliderFloat("Angle", &shade_info.m_refraction_angle, 0.1f, 1.0f);
			ImGui::PopID();
		}

		ImGui::Checkbox("Caustic", &shade_info.m_do_caustic);
		if (shade_info.m_do_caustic)
		{
			ImGui::PushID(3);
			ImGui::SameLine(); if (ImGui::Button("R"))shade_info.m_caustic_interval = { 0.0f, 1.5f }, shade_info.m_caustic_power = { 20.0f };
			ImGui::SliderFloat("Power", &shade_info.m_caustic_power, 1.0f, 64.0f);
			ImGui::SliderFloat("MinFactor", &shade_info.m_caustic_interval.x, 0.0f, shade_info.m_caustic_interval.y - 0.01f);
			ImGui::SliderFloat("MaxFactor", &shade_info.m_caustic_interval.y, shade_info.m_caustic_interval.x + 0.01f, 2.0f);
			ImGui::PopID();
		}

		ImGui::Checkbox("Light Reflection", &shade_info.m_do_lighting);
		if (shade_info.m_do_lighting)
		{
			ImGui::PushID(4);
			ImGui::SameLine(); if (ImGui::Button("R"))shade_info.m_light_interval = { 0.0f, 0.75f }, shade_info.m_light_specular = { 1.0f };
			ImGui::SliderFloat("MinLight", &shade_info.m_light_interval.x, 0.0f, shade_info.m_light_interval.y - 0.01f);
			ImGui::SliderFloat("MaxLight", &shade_info.m_light_interval.y, shade_info.m_light_interval.x + 0.01f, 1.0f);
			ImGui::SliderFloat("Specular Factor", &shade_info.m_light_specular, 0.0f, 2.0f);
			ImGui::PopID();
		}

		ImGui::Checkbox("Foam", &shade_info.m_do_foam);
		ImGui::TreePop();
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

	if (shade_info.m_render_props && m_resolution == 256)
	{
		map2d<vec3> copy_vtx{ m_resolution, m_resolution };
		copy_vtx.loop([&](size_t x, size_t y, vec3)->vec3
		{
			return m_mesh.vertices[y*m_resolution + x];
		});
		vec3 real_pos = scene->m_objects[1]->m_transform.get_pos();
		real_pos.x = glm::clamp(real_pos.x, -120.f, 120.f);
		real_pos.z = glm::clamp(real_pos.z, -120.f, 120.f);

		vec2 uv = { real_pos.x+128.0f, real_pos.z+128.0f };
		vec3 w_pos = { real_pos.x, copy_vtx.get_linear(uv).y, real_pos.z };
		vec3 w_pos_front = copy_vtx.get_linear(uv + vec2(2, 0));
		vec3 w_pos_back = copy_vtx.get_linear(uv + vec2(-2, 0));
		vec3 w_pos_right = copy_vtx.get_linear(uv + vec2(0, 2));
		vec3 w_pos_left = copy_vtx.get_linear(uv + vec2(0, -2));
		w_pos.y = (w_pos.y + w_pos_back.y + w_pos_front.y + w_pos_right.y + w_pos_left.y)/5.f;

		vec3 to_right = glm::normalize(w_pos_right - w_pos_left);
		vec3 to_front = glm::normalize(w_pos_front - w_pos_back);
		vec3 up = glm::normalize(glm::cross(to_right, to_front));

		scene->m_objects[1]->m_transform.set_pos(w_pos + vec3(0, 1.0, 0));
		scene->m_objects[1]->m_transform.set_rot(glm::quatLookAt(to_front, up));
		scene->m_objects[1]->m_transform.get_model();
		scene->m_objects[1]->m_transform.m_tr.m_pos = real_pos;
	}
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