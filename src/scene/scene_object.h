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