#pragma once
#include "scene_object.h"
#include "graphics/shader_program.h"
#include <vector>
class c_scene
{
	std::vector<scene_object*> m_objects;
	std::string m_scene_name{"scene"};
	bool load_scene(std::string);

public:
	bool init();
	void update();
	void draw(Shader_Program*);
	void shutdown();
	void drawGUI();

	friend class c_renderer;
	friend class c_editor;
};
extern c_scene * scene;
