#pragma once
#include "scene_object.h"
#include <vector>
class c_scene
{
	std::vector<scene_object*> m_objects;

public:
	bool init();
	void update();
	void shutdown();
	friend class c_renderer;
};
extern c_scene * scene;
