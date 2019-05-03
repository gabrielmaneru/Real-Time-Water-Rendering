#include "scene.h"
#include "shape.h"
c_scene * scene = new c_scene;
bool c_scene::init()
{
	//Load Scene:
	Plane* plane = new Plane(-0.5f);
	m_objects.push_back(plane);

	Sphere* sphere = new Sphere({ 0.0f, 0.0f, 0.0f }, 0.2f);
	m_objects.push_back(sphere);
	sphere = new Sphere({ 0.3f, 0.0f, 0.0f }, 0.2f);
	m_objects.push_back(sphere);
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
