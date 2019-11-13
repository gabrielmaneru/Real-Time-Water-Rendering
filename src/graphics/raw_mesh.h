/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	raw_mesh.h
Purpose: Basic mesh handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <vector>
#include <glm/glm.h>
struct raw_mesh
{
	void load();
	void free();
	void draw();
	void compute_normals();

	unsigned int m_vao{ 0 };
	unsigned int m_vtx{ 0 };
	unsigned int m_idx{ 0 };
	unsigned int m_norm{ 0 };
	unsigned int m_uvs{ 0 };
	
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> uv_coord;
	std::vector<unsigned> faces;
};