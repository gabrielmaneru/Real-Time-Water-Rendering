/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	transform3d.h
Purpose: Transform Component
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once

#include <glm/glm.h>
struct transform3d
{
	const vec3& get_pos()const { return m_tr.m_pos; }
	const vec3& get_scl()const { return m_tr.m_scl; }
	const vec3& get_rot()const { return m_tr.m_rot; }
	mat4 get_model()const { return m_tr.get_model(); }

	void set_pos(const vec3& v) { m_tr.m_pos = v, m_tr.upd(); }
	void set_scl(const vec3& v) { m_tr.m_scl = v, m_tr.upd(); }
	void set_rot(const vec3& v) { m_tr.m_rot = v, m_tr.upd(); }
	void set_tr(const vec3& pos, const vec3& scl, const vec3& rot = vec3{ .0f, .0f, .0f })
	{
		m_tr.m_pos = pos, m_tr.m_scl = scl, m_tr.m_rot = rot, m_tr.upd();
	}

	void add_parent(const mat4& model_parent) { m_tr.parent = model_parent * m_tr.parent; }

	struct Transform
	{
		vec3 m_pos{ 0,0,0 };
		vec3 m_scl{ 1,1,1 };
		vec3 m_rot{ 0,0,0 };
		mutable mat4 m_model;
		void upd() { should_update = true; }
		mutable bool should_update = true;
		mat4 get_model()const;
		mat4 parent = mat4{ 1.0f };
	} m_tr;
};