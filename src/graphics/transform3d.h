/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	transform3d.h
Purpose: Transform Component
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once

#include <glm/glm.h>
struct transform3d
{
	const vec3& get_pos()const { return m_tr.m_pos; }
	const vec3& get_scl()const { return m_tr.m_scl; }
	const quat& get_rot()const { return m_tr.m_rot; }
	mat4 get_model()const { return m_tr.get_model(); }

	void set_pos(const vec3& v) { m_tr.m_pos = v, m_tr.upd(); }
	void set_scl(const vec3& v) { m_tr.m_scl = v, m_tr.upd(); }
	void set_rot(const quat& v) { m_tr.m_rot = v, m_tr.upd(); }
	void set_tr(const vec3& pos, const vec3& scl, const quat& rot = { 1.f, .0f, .0f, .0f })
	{
		m_tr.m_pos = pos, m_tr.m_scl = scl, m_tr.m_rot = rot, m_tr.upd();
	}

	void add_parent(const mat4& model_parent) { m_tr.parent = model_parent * m_tr.parent; }

	struct Transform
	{
		vec3 m_pos{ 0.f, 0.f, 0.f };
		vec3 m_scl{ 1.f };
		quat m_rot{ 1.f, 0.f, 0.f, 0.f };
		mutable mat4 m_model;
		mutable mat4 m_prev_model;
		void upd() { should_update = true; }
		mutable bool should_update = true;
		mat4 get_model()const;
		void save_prev()const { m_prev_model = m_model; }
		mat4 get_prev_model()const { return m_prev_model; };
		mat4 parent = mat4{ 1.0f };
	} m_tr;
};