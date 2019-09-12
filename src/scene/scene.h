#pragma once
#include "scene_object.h"
#include "graphics/shader_program.h"
#include <vector>
class c_scene
{
	std::vector<scene_object*> m_objects;

public:
	bool init();
	void update();
	void draw(Shader_Program*);
	void shutdown();
	friend class c_renderer;
	friend class c_editor;
};
extern c_scene * scene;
