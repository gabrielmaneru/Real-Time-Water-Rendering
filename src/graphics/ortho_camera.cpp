#include "ortho_camera.h"

ortho_camera::ortho_camera(vec3 eye, vec3 front, vec3 up, vec4 view_rect)
	:camera( eye,front,up ), m_view_rect(view_rect) {}

void ortho_camera::update()
{
	m_view = glm::lookAt(m_eye, m_eye+m_front, m_up);
	m_proj = glm::ortho<float>(m_view_rect.x, m_view_rect.y, m_view_rect.z, m_view_rect.w);
}