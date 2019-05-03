#pragma once
#include <graphics\renderable.h>
struct scene_object : public renderable
{
	virtual ~scene_object() = default;
	virtual void enter() {};
	virtual void update() {};
	virtual void exit() {};
};