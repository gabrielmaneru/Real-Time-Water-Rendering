#include "mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures)
	: m_VAO(0), m_VBO(0), m_EBO(0), m_vertices(0), m_indices(0), m_textures(0)
{
	m_vertices = vertices;
	m_indices = indices;
	m_textures = textures;

	load();
}

Mesh::~Mesh()
{
	// Empty
}

void Mesh::load()
{
	// Generate buffers and arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	// Bind the buffers
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	// Copy data into the buffers
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

	// Vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Vertex normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);

	// Texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, textureCoords)));
	glEnableVertexAttribArray(2);

	// Tangents
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
	glEnableVertexAttribArray(3);

	// Bitangents
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bitangent)));
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw(Shader_Program* shader)const
{
	// The current diffuse and specular maps

	GLuint diffuseNr = 1, specularNr = 1, aoNr = 1, normalNr = 1, roughnessNr = 1;

	// Draw all textures
	for (size_t i = 0; i < m_textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + (GLenum)i);

		// Set the texture in the shader
		if (m_textures[i].m_type == Texture::e_texture_type::DIFFUSE) {
			shader->set_uniform(("texture_diffuse" + std::to_string(diffuseNr)).c_str(), static_cast<int>(i));
		}
		//else if (m_textures[i].m_type == Texture::e_texture_type::METALNESS) {
		//	shader->set_uniform(("texture_metalness" + std::to_string(specularNr++)).c_str(), static_cast<int>(i));
		//}
		//else if (m_textures[i].m_type == Texture::e_texture_type::AO) {
		//	shader->set_uniform(("texture_ao" + std::to_string(aoNr++)).c_str(), static_cast<int>(i));
		//}
		//else if (m_textures[i].m_type == Texture::e_texture_type::NORMAL) {
		//	shader->set_uniform(("texture_normal" + std::to_string(normalNr++)).c_str(), static_cast<int>(i));
		//}
		//else if (m_textures[i].m_type == Texture::e_texture_type::ROUGHNESS) {
		//	shader->set_uniform(("texture_roughness" + std::to_string(roughnessNr++)).c_str(), static_cast<int>(i));
		//}

		// Bind the texture
		glBindTexture(GL_TEXTURE_2D, m_textures[i].m_id);
	}
	
	// Draw mesh
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}