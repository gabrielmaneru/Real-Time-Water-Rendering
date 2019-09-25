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
class scene_object : public renderable
{
public:
	scene_object(std::string mesh, transform3d tr = {});
	virtual ~scene_object() = default;
	virtual void enter() {};
	virtual void update() {};
	virtual void draw(Shader_Program*);
	virtual void exit() {};
};