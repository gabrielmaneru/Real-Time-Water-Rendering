#pragma once
#include "scene_object.h"

struct ik_bone
{
	ik_bone(ik_bone*p):m_parent(p){}
	mat4 get_model()const;
	void set_model(const mat4&);
	mat4 get_concat_rotation()const;
	vec3 get_head()const;
	vec3 get_tail()const;

	quat m_rotation{};
	float m_length{1.0f};
	ik_bone * m_parent{ nullptr };
	std::vector<ik_bone *> m_children;
};

class ik_chain : public scene_object
{
public:
	ik_chain(transform3d tr = {}, size_t start_count=2u);
	void draw(Shader_Program*)override;
	void draw_GUI()override;
	void run_2_bone_ik();
	bool run_ccd();

	int m_iterations{ 1 };
	int m_solver{ 0 };
	bool m_active{false};
	ik_bone* m_root;
	vec3 m_end_effector;
	size_t m_selected{0};
	std::vector<ik_bone*> m_bones;
};