#include "vectorial_camera.h"
#include <platform/window_manager.h>
#include <platform/window.h>
#include <GLFW/glfw3.h>
void vectorial_camera::update(float xOff, float yOff)
{
	constexpr float MOUSE_SENSITIVITY{ 0.2f };
	constexpr float MOUSE_SPEED{ 0.05f };

	// Check for Mouse Imput
	if (window::mouse_but_right_pressed)
	{
		float speed = MOUSE_SPEED * (window_manager->is_key_down(GLFW_KEY_LEFT_SHIFT) ? 5.0f : 1.0f);

		if (window_manager->is_key_down(GLFW_KEY_W))
			m_eye += m_front * speed;
		if (window_manager->is_key_down(GLFW_KEY_S))
			m_eye -= m_front * speed;
		if (window_manager->is_key_down(GLFW_KEY_D))
			m_eye += m_right * speed;
		if (window_manager->is_key_down(GLFW_KEY_A))
			m_eye -= m_right * speed;
		if (window_manager->is_key_down(GLFW_KEY_Q))
			m_eye += m_up * speed;
		if (window_manager->is_key_down(GLFW_KEY_E))
			m_eye -= m_up * speed;

		// Apply Offset
		xOff *= MOUSE_SENSITIVITY;
		yOff *= MOUSE_SENSITIVITY;
		m_yaw += xOff;
		m_pitch += yOff;

		// Check Bounds
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		update_cam_vectors();
	}

	// Update Camera Matrices
	m_view = glm::lookAt(m_eye, m_eye + m_front, m_up);
	float aspect = window_manager->get_width() / (float)window_manager->get_height();
	m_proj = glm::perspective(m_fovY, aspect, m_near, m_far);
}

void vectorial_camera::update_cam_vectors()
{
	// Recompute Front
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);

	// Recompute Right and Up
	m_right = glm::normalize(glm::cross(m_front, m_worldup));
	m_up = glm::normalize(glm::cross(m_right, m_front));
}
