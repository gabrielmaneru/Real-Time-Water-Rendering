/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	mesh.cpp
Purpose: Mesh manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "mesh.h"
#include "gl_error.h"


Mesh::~Mesh()
{
	glDeleteBuffers(1, &m_vertexbuffer);
	glDeleteBuffers(1, &m_normalbuffer);
	glDeleteBuffers(1, &m_uvbuffer);
	glDeleteBuffers(1, &m_tangentbuffer);
	glDeleteBuffers(1, &m_bitangentbuffer);
	glDeleteBuffers(1, &m_wbonesbuffer);
	glDeleteBuffers(1, &m_bonesbuffer);
	glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::load()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_vertexbuffer);
	glGenBuffers(1, &m_normalbuffer);
	glGenBuffers(1, &m_uvbuffer);
	glGenBuffers(1, &m_tangentbuffer);
	glGenBuffers(1, &m_bitangentbuffer);
	glGenBuffers(1, &m_wbonesbuffer);
	glGenBuffers(1, &m_bonesbuffer);
	glGenBuffers(1, &m_indexbuffer);

	// Vertex positions
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.position.size() * sizeof(vec3), m_vertices.position.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));


	// Vertex normals
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.normal.size() * sizeof(vec3), m_vertices.normal.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

	// Texture coordinates
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.uv.size() * sizeof(vec2), m_vertices.uv.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0));

	// Tangents
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_tangentbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.tangent.size() * sizeof(vec3), m_vertices.tangent.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(3));
	GL_CALL(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

	// Bitangents
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_bitangentbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.bitangent.size() * sizeof(vec3), m_vertices.bitangent.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(4));
	GL_CALL(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

	// WBones
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_wbonesbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.wbones.size() * sizeof(vec4), m_vertices.wbones.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(5));
	GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (void*)0));

	// Bones
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_bonesbuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.bones.size() * sizeof(ivec4), m_vertices.bones.data(), GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(6));
	GL_CALL(glVertexAttribPointer(6, 4, GL_INT, GL_FALSE, 0, (void*)0));

	// Indices
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexbuffer));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW));

	GL_CALL(glBindVertexArray(0));
	GL_CALL(glBindVertexArray(m_VAO));
}

void Mesh::draw(Shader_Program* shader)const
{
	// Draw mesh
	GL_CALL(glBindVertexArray(m_VAO));
	switch (m_primitive)
	{
	case Mesh::tri:
		GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0));
		break;
	case Mesh::quad:
		GL_CALL(glDrawElements(GL_QUADS, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0));
		break;
	}
	GL_CALL(glBindVertexArray(0));
}

VertexBuffer::VertexBuffer(size_t s)
{
	position.resize(s);
	normal.resize(s);
	uv.resize(s);
	tangent.resize(s);
	bitangent.resize(s);
	bones.resize(s, ivec4(-1));
	wbones.resize(s, vec4(0.0f));
}
