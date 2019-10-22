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
#include <graphics/curve.h>
struct interpolator
{
	bool m_active{ true };
	bool m_playback{ false };
	bool m_playback_state{ true };
	double m_time{ 0.0 };
	double m_speed{ 1.0 };
	void update(double max_t);
	void draw_GUI();
};
struct animator : public interpolator
{
	void draw_GUI();
	int m_current_animation{ 0 };
};
struct curve_interpolator : public interpolator
{
	void draw_GUI();
	const curve_base* m_actual_curve{ nullptr };
};

class scene_object : public renderable
{
public:
	scene_object(std::string mesh, transform3d tr, animator * anim, curve_interpolator* curve);
	virtual ~scene_object();
	virtual void enter() {};
	virtual void update() {};
	virtual void draw(Shader_Program*);
	virtual void exit() {};

	void draw_GUI();
	void update_parent_curve();

	bool m_tesselate{false};
	animator* m_animator{ nullptr };
	curve_interpolator* m_curve{ nullptr };

};