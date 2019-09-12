#include "scene.h"
#include "graphics/renderer.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	m_objects.push_back(new scene_object);
	m_objects.push_back(new scene_object);
	m_objects.push_back(new scene_object);
	m_objects.push_back(new scene_object);
	m_objects.push_back(new scene_object);
	m_objects[0]->m_transform.set_pos({ 0.0, 0.0, 1.0 });
	m_objects[0]->m_model = renderer->get_model(renderer->cube);
	m_objects[1]->m_transform.set_pos({ 1.0, 0.0, 0.0 });
	m_objects[1]->m_model = renderer->get_model(renderer->octohedron);
	m_objects[2]->m_transform.set_pos({ 0.0, 0.0, -1.0 });
	m_objects[2]->m_model = renderer->get_model(renderer->quad);
	m_objects[3]->m_transform.set_pos({ -1.0, 0.0, 0.0 });
	m_objects[3]->m_model = renderer->get_model(renderer->sphere);
	m_objects[4]->m_model = renderer->get_model(renderer->sponza);
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
