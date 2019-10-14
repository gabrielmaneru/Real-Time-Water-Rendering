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

struct BoneData
{
	mat4 m_offset;
	mat4 m_final_transform;
};

struct Mesh
{
	Mesh(size_t s) :m_vertices(s) {}
	~Mesh();
	void draw(Shader_Program* shader)const;
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

	int m_material_idx;
	VertexBuffer m_vertices;
	std::vector<GLuint> m_indices;
	std::vector<Texture> m_textures;
	enum e_prim { tri, quad } m_primitive;
	std::vector<BoneData> m_bones;
	std::map<std::string, size_t> m_bone_mapping;
};