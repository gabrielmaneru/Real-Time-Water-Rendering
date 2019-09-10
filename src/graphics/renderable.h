#pragma once
#include "transform3d.h"
#include "mesh.h"
class renderable
{
public:
	transform3d m_transform;
	MeshReference m_mesh_ref;
};