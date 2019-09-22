#include "scene.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	{
		transform3d tr;
		tr.set_scl(vec3(0.1f, 0.1f, 0.1f));
		m_objects.push_back(new scene_object("sponza", tr));
	}

	{
		transform3d tr;
		tr.set_pos(vec3(-90.f, 30.f, -3.f));
		tr.set_scl(vec3(0.5f, 0.5f, 0.5f));
		m_objects.push_back(new scene_object("sphere", tr));
	}

	{
		transform3d tr;
		tr.set_pos(vec3(-13.f, 11.f, -4.f));
		tr.set_rot(vec3(-90.f, 0.f, 0.f));
		tr.set_scl(vec3(.025f));
		m_objects.push_back(new scene_object("phoenix", tr));
	}
	return true;
}

void c_scene::update()
{
}

void c_scene::draw(Shader_Program * shader)
{
	for (auto p_obj : m_objects)
		p_obj->draw(shader);
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
		delete p_obj;
	m_objects.clear();
}
