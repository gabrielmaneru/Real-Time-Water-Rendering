#pragma once
#include <glm/glm.h>

struct vectorial_camera
{
	void update(float xOff, float yOff);
	void update_cam_vectors();
	void update_cam_vectors(vec3 front);

	const float min_near = 0.0001f;
	const float max_far = 1000.0f;

	float m_fovY{ glm::pi<float>() / 4 };
	float m_near{ min_near };
	float m_far{ max_far };

	mat4 m_proj;
	mat4 m_view;

	vec3 m_eye{0.0f, 0.0f, 0.0f};
	vec3 m_front{};
	vec3 m_right{};
	vec3 m_up{0.0f, 1.0f, 0.0f};

	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	vec3 m_worldup{ 0.0f, 1.0f, 0.0f };
};