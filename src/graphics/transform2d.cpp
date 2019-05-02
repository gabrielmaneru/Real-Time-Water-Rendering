#include "transform2d.h"

mat4 transform2d::Transform::get_model() const
{
	if (should_update)
	{
		m_model = glm::translate(mat4(1.0f), vec3(m_pos, 0.0f));

		m_model = glm::rotate(m_model, glm::radians(m_rot), glm::vec3(0.0f, 0.0f, 1.0f));

		m_model = glm::scale(m_model, vec3(m_scl, 1.0f));
		should_update = false;
	}
	return m_model;
}