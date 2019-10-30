/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene.h
Purpose: Scene Manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "scene_object.h"
#include "light.h"
#include "decal.h"
#include "graphics/shader_program.h"
#include <vector>
class c_scene
{
	std::vector<scene_object*> m_objects;
	std::vector<light*> m_lights;
	std::vector<decal*> m_decals;
	std::string m_scene_name{"scene"};
	bool m_animated_scene{false};
	int m_num_lights = 40;
	bool load_scene(std::string);

public:
	bool init();
	void update();
	void draw_objs(Shader_Program*);
	void draw_lights(Shader_Program*);
	void draw_decals(Shader_Program*);
	void draw_debug_lights(Shader_Program*);
	void draw_debug_curves(Shader_Program*);
	void shutdown();
	void drawGUI();

	friend class c_renderer;
	friend class c_editor;
};
extern c_scene * scene;
