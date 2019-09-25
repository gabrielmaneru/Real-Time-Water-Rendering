#pragma once
#include "camera.h"
class scene_object;
class vectorial_camera : public camera
{
public:
	vectorial_camera(vec3 eye = def_cam.eye,
		vec3 front = def_cam.front,
		vec3 up = def_cam.up,
		float yaw = def_cam.yaw,
		float pitch = def_cam.pitch);
	void update()override;
	void use_target(const scene_object *);
	void release_target();
	const scene_object * get_target() { return m_target; }

	float m_fov{ 90.0f };
	float m_near{ .1f };
	float m_far{ 1000.0f };

private:
	void update_free_mode();
	void update_target_mode();
	void update_cam_vectors();
	void update_cam_vectors(vec3 front);
	vec3 m_right;
	vec3 m_worldup{ def_cam.up };
	
	// Target Mode
	const scene_object * m_target{ nullptr };
	float m_alpha = { 0.0f };
	float m_beta = { 0.0f };
	float m_dist = { 0.0f };

	// Free Mode
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
};