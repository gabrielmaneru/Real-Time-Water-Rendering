#pragma once
#include <graphics\renderer.h>
struct scene_object
{
	virtual ~scene_object() = default;
	virtual void enter();
	virtual void update();
	virtual void draw();
	virtual void exit();
};