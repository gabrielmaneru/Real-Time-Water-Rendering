#pragma once

#include <glm/glm.h>
struct transform2d
{
	const vec2& get_pos() { return m_tr.m_pos; }
	const vec2& get_scl() { return m_tr.m_scl; }
	const float& get_rot() { return m_tr.m_rot; }

	void set_pos(const vec2& v) { m_tr.m_pos = v, m_tr.upd(); }
	void set_scl(const vec2& v) { m_tr.m_scl = v, m_tr.upd(); }
	void set_rot(const float& v) { m_tr.m_rot = v, m_tr.upd(); }
	void set_tr(const vec2& pos, const vec2& scl, const float& rot = 0.0f)
	{
		m_tr.m_pos = pos, m_tr.m_scl = scl, m_tr.m_rot = rot, m_tr.upd();
	}

	struct Transform
	{
		vec2 m_pos{ 0.0f, 0.0f };
		vec2 m_scl{ 1.0f, 1.0f };
		float m_rot{ 0.0f };

		mutable mat4 m_model;
		void upd() { should_update = true; }
		mutable bool should_update = true;
		mat4 get_model()const;
	} m_tr;
};