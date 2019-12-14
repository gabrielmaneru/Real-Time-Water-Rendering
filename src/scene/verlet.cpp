#include "verlet.h"

void particle::integrate(float dt, float inertia)
{
	if (!fixed)
	{
		prev_pos = pos + inertia * (pos - prev_pos) + acc * (dt * dt);
		std::swap(pos, prev_pos);
	}
	acc = { .0f };
}

joint::joint(particle & a, particle & b)
	:m_a(a), m_b(b), m_length(glm::length(a.pos-b.pos)) {}

void joint::relax(float restitution)
{
	vec3 d = m_b.pos - m_a.pos;
	float d_len = glm::length(d);
	float diff = (d_len - m_length) / d_len * restitution;

	if(!m_a.fixed)
	{
		if (!m_b.fixed)
		{
			m_a.pos += d * 0.5f * diff;
			m_b.pos -= d * 0.5f * diff;
		}
		else
			m_a.pos += d * diff;
	}
	else if (!m_b.fixed)
		m_b.pos -= d * diff;
}

void verlet_system::update(float dt)
{
	for (auto& p : m_particles)
	{
		p.acc += m_passive_acc;
		p.integrate(dt, m_inertia);
	}

	for (size_t i = 0; i < m_relax_iterations; i++)
		for (auto& j : m_joints)
			j.relax(m_restitution);
}

#include <graphics/renderer.h>
#include <graphics/model.h>
#include <platform/window.h>
#include <imgui/imgui.h>
cloth::cloth(transform3d tr)
	:scene_object("", tr, nullptr, nullptr)
{
	int res = 50;
	m_mesh.build_plane(res, 1.0f);
	m_mesh.compute_normals();

	m_verlet.m_particles = { m_mesh.vertices.begin(), m_mesh.vertices.end() };

	m_verlet.m_particles[0].fixed =
	m_verlet.m_particles[res*1/3-1].fixed =
	m_verlet.m_particles[res*2/3-1].fixed =
	m_verlet.m_particles[res-1].fixed = true;

	for (int y = 0; y < res; y++)
		for (int x = 0; x < res - 1; x++)
		{
			m_verlet.m_joints.emplace_back(joint{ m_verlet.m_particles[y*res+x],m_verlet.m_particles[y*res+(x+1)] });
			m_verlet.m_joints.emplace_back(joint{ m_verlet.m_particles[x*res+y],m_verlet.m_particles[(x+1)*res+y] });
		}
}

void cloth::draw(Shader_Program * shader)
{
	m_verlet.update(static_cast<float>(1.0/window::frameTime));
	m_mesh.vertices = { m_verlet.m_particles.begin(), m_verlet.m_particles.end() };
	m_mesh.load();

	Model::m_def_materials[1]->set_uniform(shader);
	shader->set_uniform("M", m_transform.m_tr.get_model());
	m_mesh.draw();
}

void cloth::draw_GUI()
{
	ImGui::DragInt("Relax It", &m_verlet.m_relax_iterations, 1.0f, 1, 999);
	ImGui::SliderFloat("Inertia", &m_verlet.m_inertia, 0.0f, 1.0f);
	ImGui::SliderFloat("Restitution", &m_verlet.m_restitution, 0.0f, 1.0f);
	ImGui::DragFloat3("Passive Acc", &m_verlet.m_passive_acc.x);
}

