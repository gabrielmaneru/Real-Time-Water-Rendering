#pragma once
#include "scene_object.h"
#include <graphics/raw_mesh.h>

struct Particle
{
	vec3 position;
	vec3 prev_position;
};
struct Joint
{

};
struct verlet_system
{
	std::vector<Particle> m_particles;
	std::vector<Joint> m_joints;
};

class cloth : public scene_object
{
	raw_mesh m_mesh;
	verlet_system m_verlet;

public:
	cloth(transform3d tr = {});
	void draw(Shader_Program*)override;
	void draw_GUI()override;
};