/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	ortho_camera.cpp
Purpose: Basic orthogonal camera
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "ortho_camera.h"

ortho_camera::ortho_camera(vec3 eye, vec3 front, vec3 up, vec4 view_rect)
	:camera( eye,front,up ), m_view_rect(view_rect) {}

void ortho_camera::update()
{
	m_view = glm::lookAt(m_eye, m_eye+m_front, m_up);
	m_proj = glm::ortho<float>(m_view_rect.x, m_view_rect.y, m_view_rect.z, m_view_rect.w);
}