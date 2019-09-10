#pragma once
#include <graphics/renderable.h>
#include <graphics/shader_program.h>
class scene_object : public renderable
{
public:
	virtual ~scene_object() = default;
	virtual void enter() {};
	virtual void update() {};
	virtual void draw(Shader_Program*);
	virtual void exit() {};
};