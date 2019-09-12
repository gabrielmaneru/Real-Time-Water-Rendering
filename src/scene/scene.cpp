#include "scene.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	m_objects.push_back(new scene_object("sponza"));
	transform3d tr;
	tr.set_pos(vec3(-900.f, 300.f, -30.f));
	tr.set_scl(vec3(5.f, 5.f, 5.f));
	m_objects.push_back(new scene_object("sphere", tr));
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
