#pragma once
#include "scene_object.h"

struct ik_bone
{
	ik_bone(ik_bone*p):m_parent(p){}
	vec3 get_relative_pos();
	quat get_relative_rotation();

	quat m_rotation{};
	float m_length{1.0f};
	ik_bone * m_parent{ nullptr };
	std::vector<ik_bone *> m_children;
};

class ik_chain : public scene_object
{
public:
	ik_bone* m_root;
	std::vector<ik_bone*> m_bones;
	size_t m_selected{0};

	ik_chain(transform3d tr = {}, size_t start_count=2u);
	void draw(Shader_Program*)override;
	void draw_GUI()override;
};