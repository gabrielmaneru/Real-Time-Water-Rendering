/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	transform3d.cpp
Purpose: Transform Component
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "transform3d.h"

mat4 transform3d::Transform::get_model() const
{
	if (should_update)
	{
		m_model = glm::translate(mat4(1.0f), m_pos);

		m_model = m_model*glm::mat4_cast(m_rot);

		m_model = glm::scale(m_model, vec3{ m_scl });
		should_update = false;
	}
	return parent * m_model;
}
