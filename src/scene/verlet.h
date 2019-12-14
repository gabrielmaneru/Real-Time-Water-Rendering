#pragma once
#include <glm/glm.h>
#include <vector>

struct particle
{
	particle(const vec3& initial_pos)
		:acc({ 0.0f, -9.8f, 0.0f }), pos(initial_pos), prev_pos(initial_pos), fixed(false) {}
	operator vec3()const { return pos; }
	void integrate(float dt, float inertia);

	vec3 acc;
	vec3 pos;
	vec3 prev_pos;
	bool fixed;
};
struct joint
{
	joint(particle& a, particle& b);
	void relax(float restitution);

	particle &m_a, &m_b;
	float m_length;
};
struct verlet_system
{
	void update(float dt);

	int m_relax_iterations{ 7 };
	float m_inertia{ 0.95f };
	float m_restitution{ 0.95f };
	vec3 m_passive_acc{ 0.0f, -100.0f, 0.0f };
	std::vector<particle> m_particles;
	std::vector<joint> m_joints;
};

#include "scene_object.h"
#include <graphics/raw_mesh.h>
class cloth : public scene_object
{
	raw_mesh m_mesh;
	verlet_system m_verlet;

public:
	cloth(transform3d tr);
	void draw(Shader_Program*)override;
	void draw_GUI()override;
};