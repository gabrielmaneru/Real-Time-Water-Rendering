#pragma once
#include "shader_program.h"
#include "texture.h"
#include <glm/glm.h>
#include <GL/gl3w.h>
#include <vector>

struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 textureCoords;
	vec3 tangent;
	vec3 bitangent;
};

struct Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures);
	~Mesh();

	void draw(Shader_Program* shader)const;

	GLuint getVAO() const { return m_VAO; }
	std::vector<GLuint> getIndices() const { return m_indices; }
private:
	void load();

	GLuint m_VAO, m_VBO, m_EBO;

	std::vector<Vertex> m_vertices;
	std::vector<GLuint> m_indices;
	std::vector<Texture> m_textures;
};