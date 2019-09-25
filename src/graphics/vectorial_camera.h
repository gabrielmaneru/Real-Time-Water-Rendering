#pragma once
#include "camera.h"
class vectorial_camera : public camera
{
public:
	vectorial_camera(vec3 eye = def_cam.eye,
		vec3 front = def_cam.front,
		vec3 up = def_cam.up,
		float yaw = def_cam.yaw,
		float pitch = def_cam.pitch);
	void update()override;

	float m_fov{ 90.0f };
	float m_near{ .1f };
	float m_far{ 1000.0f };

private:
	void update_cam_vectors();
	
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	vec3 m_worldup{ def_cam.up };
	vec3 m_right;
};