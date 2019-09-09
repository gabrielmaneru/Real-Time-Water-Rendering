#pragma once
#include <glm/glm.h>
#include <vector>
#include <string>
#include <fstream>
#include <GL/gl3w.h>

struct Mesh
{
	Mesh()=default;
	Mesh(const std::string&);
	~Mesh();

	bool load_obj(const std::string&);
	void bind();
	void use()const;
	unsigned idx_count()const;

	GLuint m_VAO;
	GLuint m_vertexbuffer;
	GLuint m_uvbuffer;
	GLuint m_normalbuffer;
	GLuint m_indexbuffer;

	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;
	std::vector<vec2> m_uvs;
	std::vector<unsigned short> m_index;

	std::string m_name;
	bool has_normals{ false };
	bool m_binded{ false };

	vec2 parse_vec2(std::ifstream& file);
	vec3 parse_vec3(std::ifstream& file);
	void parse_face(std::vector<vec2>& temp_uvs, std::vector<vec3>& temp_normals, std::vector<vec3>& temp_vertices, std::vector<vec2>& in_uvs, std::vector<vec3>& in_normals, std::vector<vec3>& in_vertices, std::ifstream& file);
	void fill_idx_buffers(std::vector<vec3> & in_vertices, std::vector<vec2> & in_uvs, std::vector<vec3> & in_normals);
};