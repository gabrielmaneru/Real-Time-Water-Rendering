#include "scene.h"
#include "graphics/renderer.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	m_objects.push_back(new scene_object());
	m_objects[0]->m_model = renderer->get_model(renderer->sponza);
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
