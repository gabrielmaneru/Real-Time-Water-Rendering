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

		m_model = glm::rotate(m_model, glm::radians(m_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_model = glm::rotate(m_model, glm::radians(m_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_model = glm::rotate(m_model, glm::radians(m_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));

		m_model = glm::scale(m_model, m_scl);
		should_update = false;
	}
	return parent * m_model;
}
