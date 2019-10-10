/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	mesh.h
Purpose: Mesh manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "shader_program.h"
#include "texture.h"
#include <glm/glm.h>
#include <GL/gl3w.h>
#include <vector>

struct VertexBuffer
{
	VertexBuffer(size_t s);
	VertexBuffer(const VertexBuffer&) = default;
	std::vector<vec3> position;
	std::vector<vec3> normal;
	std::vector<vec2> uv;
	std::vector<vec3> tangent;
	std::vector<vec3> bitangent;
	std::vector<vec4> wbones;
	std::vector<ivec4> bones;
};

struct Mesh
{
public:
	enum e_prim
	{
		tri, quad
	}m_primitive;
	VertexBuffer m_vertices;
	Mesh(const VertexBuffer& vertices, const std::vector<GLuint>& indices, int material_idx, e_prim);
	~Mesh();

	void draw(Shader_Program* shader)const;
	int m_material_idx;

private:
	void load();

	GLuint m_VAO{ 0 };
	GLuint m_vertexbuffer{ 0 };
	GLuint m_normalbuffer{ 0 };
	GLuint m_uvbuffer{ 0 };
	GLuint m_tangentbuffer{ 0 };
	GLuint m_bitangentbuffer{ 0 };
	GLuint m_wbonesbuffer{ 0 };
	GLuint m_bonesbuffer{ 0 };
	GLuint m_indexbuffer{ 0 };

	std::vector<GLuint> m_indices;
	std::vector<Texture> m_textures;
	std::vector<mat4> m_bones;
};