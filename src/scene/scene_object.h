/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	scene_object.h
Purpose: Object base class
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <graphics/renderable.h>
#include <graphics/shader_program.h>
struct animator
{
	bool m_active{ true };
	bool m_playback{ false };
	bool m_playback_state{ true };
	double m_time{ 0.0 };
	int m_current_animation{ 0 };
	void draw_GUI();
};

class scene_object : public renderable
{
public:
	scene_object(std::string mesh, transform3d tr = {}, animator * anim = nullptr);
	animator* m_animator{nullptr};
	virtual ~scene_object() { if(m_animator) delete m_animator; }
	virtual void enter() {};
	virtual void update() {};
	virtual void draw(Shader_Program*);
	virtual void exit() {};
};