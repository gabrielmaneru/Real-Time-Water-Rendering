#pragma once
#include "scene_object.h"
#include <graphics/raw_mesh.h>

class cloth : public scene_object
{
	raw_mesh m_mesh;

public:
	cloth(transform3d tr = {});
	void draw(Shader_Program*)override;
	void draw_GUI()override;
};