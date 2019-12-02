/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	vectorial_camera.h
Purpose: Perspective camera controlled using vectors
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "vectorial_camera.h"
#include <scene/scene_object.h>
#include <platform/window_manager.h>
#include <platform/window.h>
#include <GLFW/glfw3.h>

vectorial_camera::vectorial_camera(vec3 eye, vec3 front, vec3 up, float yaw, float pitch)
	:camera(eye, front, up), m_yaw(yaw), m_pitch(pitch)
{
	update_cam_vectors();
}

void vectorial_camera::update()
{
	if (m_target)
		update_target_mode();
	else
		update_free_mode();

	// Update Projection
	float aspect = window_manager->get_width() / (float)window_manager->get_height();
	m_proj = glm::perspective(glm::radians(m_fov), aspect, m_near, m_far);
}

void vectorial_camera::use_target(const scene_object * target)
{
	m_target = target;

	vec3 delta = m_target->m_transform.get_real_pos() - m_eye;
	m_dist = glm::length(delta);
	delta = -normalize(delta);
	m_beta = glm::asin(delta.y);
	m_alpha = glm::asin(delta.z);
	if (delta.x < 0.0f)
		m_alpha += 2 * (glm::pi<float>()*0.5f - m_alpha);
}

void vectorial_camera::release_target()
{
	vec3 delta = normalize(m_target->m_transform.get_real_pos() - m_eye);
	m_target = nullptr;

	m_pitch = glm::degrees(glm::asin(delta.y));
	m_yaw = glm::degrees(glm::asin(delta.z));
	if (delta.x < 0.0f)
		m_yaw += 2 * (90 - m_yaw);
}

constexpr float MOUSE_SENSITIVITY{ 0.25f };
constexpr float MOUSE_SPEED{ 0.5f };
constexpr float ROTATION_SPEED{ 0.005f };
void vectorial_camera::update_free_mode()
{

	// Check for Mouse Imput
	if (window::mouse_but_right_pressed)
	{
		float speed = MOUSE_SPEED * (window_manager->is_key_down(GLFW_KEY_LEFT_SHIFT) ? 10.0f : 1.0f);

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
		m_yaw += window::mouse_offset[0] * MOUSE_SENSITIVITY;
		m_pitch += window::mouse_offset[1] * MOUSE_SENSITIVITY;

		// Check Bounds
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

	}
	update_cam_vectors();
}

void vectorial_camera::update_target_mode()
{

	// Check for Mouse Imput
	if (window::mouse_but_right_pressed)
	{
		float speed = ROTATION_SPEED * (window_manager->is_key_down(GLFW_KEY_LEFT_SHIFT) ? 10.0f : 1.0f);
		if (window_manager->is_key_down(GLFW_KEY_A))
			m_alpha += speed;
		if (window_manager->is_key_down(GLFW_KEY_D))
			m_alpha -= speed;
		if (window_manager->is_key_down(GLFW_KEY_W))
			m_beta += speed;
		if (window_manager->is_key_down(GLFW_KEY_S))
			m_beta -= speed;
		if (window_manager->is_key_down(GLFW_KEY_E))
			m_dist *= 1 + speed;
		if (window_manager->is_key_down(GLFW_KEY_Q))
			m_dist *= 1 - speed;

		// Apply Offset
		static const float mouse_speed = glm::pow(MOUSE_SENSITIVITY, 5);
		m_alpha += window::mouse_offset[0] * mouse_speed;
		m_beta -= window::mouse_offset[1] * mouse_speed;
	}
	m_beta = glm::clamp(m_beta, glm::pi<float>() * -.45f, glm::pi<float>() * .45f);
	m_dist = glm::clamp(m_dist, m_near, m_far * 0.5f);
	m_eye = m_target->m_transform.get_real_pos();
	m_eye += vec3(	m_dist * cosf(m_beta) * cosf(m_alpha),
					m_dist * sinf(m_beta),
					m_dist * cosf(m_beta) * sinf(m_alpha) );

	update_cam_vectors(glm::normalize(m_target->m_transform.get_real_pos() - m_eye));
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

	// Update View
	m_view = glm::lookAt(m_eye, m_eye + m_front, m_up);
}

void vectorial_camera::update_cam_vectors(vec3 front)
{
	m_front = glm::normalize(front);

	// Recompute Right and Up
	m_right = glm::normalize(glm::cross(m_front, m_worldup));
	m_up = glm::normalize(glm::cross(m_right, m_front));

	// Update View
	m_view = glm::lookAt(m_eye, m_eye + m_front, m_up);
}
