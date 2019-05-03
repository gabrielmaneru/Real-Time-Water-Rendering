#pragma once
#include "renderer.h"
struct renderable
{
	virtual ~renderable() = default;
	virtual void draw(c_renderer &) {};
};