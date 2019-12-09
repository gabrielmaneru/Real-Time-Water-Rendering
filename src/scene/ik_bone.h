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
	void set_tail(const vec3 &);

	quat m_rotation{};
	float m_length{1.0f};
	ik_bone * m_parent{ nullptr };
};

class ik_chain : public scene_object
{
	enum e_Solver {
		e_2BoneIK,
		e_CCD,
		e_FABRIK
	} m_solver{ e_2BoneIK };
	enum e_Status {
		e_Running,
		e_Finished,
		e_OutofReach,
		e_Failed
	} m_status{ e_Running };
	void run_solver();
	bool is_end_outofreach();
	bool is_finished();
	e_Status status_check();

public:
	ik_chain(transform3d tr = {}, size_t start_count=2u);
	void draw(Shader_Program*)override;
	void reset();
	void draw_GUI()override;
	e_Status run_2_bone_ik();
	e_Status run_ccd();
	e_Status run_FABRIK();

	struct Iteration_Info
	{
		int iteration_per_frame{50};
		int iteration_count{0};
		int iteration_maximum{ 1000 };
	} m_iterations;
	float m_epsilon{ 0.1f };
	bool m_active{false};
	ik_bone* m_root;
	vec3 m_end_effector;
	size_t m_selected{0};
	std::vector<ik_bone*> m_bones;

	friend class c_scene;
};