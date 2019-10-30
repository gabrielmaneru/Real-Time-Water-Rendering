#pragma once

#include "scene_object.h"
#include "graphics/texture.h"
class decal : public scene_object
{
	Texture m_diffuse;
	Texture m_normal;
public:
	decal(std::string diffuse, std::string normal, transform3d tr);
};