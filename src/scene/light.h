#pragma once
#include "scene_object.h"
struct light_data
{
	static vec3 m_ambient;
	static float m_att_max;
	vec3 m_diffuse{1.0f};
	vec3 m_specular{0.8f};
	vec3 m_att_factor{0.05f};
	void drawGUI();
};

class light : public scene_object
{
	void recompute_scale();
public:
	light(transform3d tr = {}, light_data ld={});
	void draw(Shader_Program*);

	light_data m_ldata;
};