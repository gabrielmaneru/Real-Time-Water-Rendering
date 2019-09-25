#pragma once
#include "scene_object.h"
#include "light.h"
#include "graphics/shader_program.h"
#include <vector>
class c_scene
{
	std::vector<scene_object*> m_objects;
	std::vector<light*> m_lights;
	std::string m_scene_name{"scene"};
	bool load_scene(std::string);

public:
	bool init();
	void update();
	void draw_objs(Shader_Program*);
	void draw_lights(Shader_Program*);
	void draw_debug_lights(Shader_Program*);
	void shutdown();
	void drawGUI();

	friend class c_renderer;
	friend class c_editor;
};
extern c_scene * scene;
