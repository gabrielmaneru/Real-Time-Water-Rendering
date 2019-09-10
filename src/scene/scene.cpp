#include "scene.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	m_objects.push_back(new scene_object());
	m_objects[0]->m_mesh_ref.validate("cube");
	return true;
}

void c_scene::update()
{
}

void c_scene::shutdown()
{
	for (auto p_obj : m_objects)
	{
		delete p_obj;
	}
	m_objects.clear();
}
