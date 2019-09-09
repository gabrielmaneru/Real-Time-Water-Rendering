#include "mesh.h"
#include <cstdio>
#include <cstring>
#include <map>

Mesh::Mesh(const std::string& filepath) : Mesh()
{
	if(!load_obj(filepath))
		throw std::string("Mesh not found: ")+filepath;
	bind();
}

Mesh::~Mesh()
{
	if (m_binded)
	{
		glDeleteBuffers(1, &m_indexbuffer);
		glDeleteBuffers(1, &m_normalbuffer);
		glDeleteBuffers(1, &m_uvbuffer);
		glDeleteBuffers(1, &m_vertexbuffer);

		glDeleteVertexArrays(1, &m_VAO);
	}
}

bool Mesh::load_obj(const std::string & filepath)
{
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;

	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;
	std::vector<vec3> temp_vertices;

	std::vector<vec2> in_uvs;
	std::vector<vec3> in_normals;
	std::vector<vec3> in_vertices;

	// Open OBJ file
	std::ifstream file(filepath);
	if (!file.is_open())
		return false;

	// Read until it breaks on EOF
	while (!file.eof())
	{
		std::string lineHeader;

		// Read the first word of the line

		file >> lineHeader;

		// Read lineHeader
		if (lineHeader == "v")
			temp_vertices.push_back(parse_vec3(file));
		else if (lineHeader == "vt")
			temp_uvs.push_back(parse_vec2(file));
		else if (lineHeader == "vn")
			temp_normals.push_back(parse_vec3(file)), has_normals = true;
		else if (lineHeader == "f")
			parse_face(temp_uvs, temp_normals, temp_vertices, in_uvs, in_normals, in_vertices, file);
		else
		{
			std::string tmp;
			std::getline(file, tmp);
		}
	}

	fill_idx_buffers(in_vertices, in_uvs, in_normals);
	m_name = filepath.substr(filepath.find_last_of('/')+1, filepath.find_last_of('.') - filepath.find_last_of('/')-1);
	return true;
}

void Mesh::bind()
{
	if (m_binded)
		return;

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_vertexbuffer);
	glGenBuffers(1, &m_uvbuffer);
	glGenBuffers(1, &m_normalbuffer);
	glGenBuffers(1, &m_indexbuffer);

	// Bind a buffer of vertices
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(vec3), &m_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// Bind a buffer for the UV coordinates
	glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(vec2), &m_uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// Bind a buffer for the indices
	glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(vec3), &m_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// Bind a buffer for the indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index.size() * sizeof(unsigned short), &m_index[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	m_binded = true;
}

void Mesh::use() const
{
	glBindVertexArray(m_VAO);
}

unsigned Mesh::idx_count() const
{
	return static_cast<unsigned>(m_index.size());
}

vec2 Mesh::parse_vec2(std::ifstream & file)
{
	std::string val;
	vec2 vec;

	file >> val;
	vec.x = std::stof(val);
	file >> val;
	vec.y = std::stof(val);
	return vec;
}

vec3 Mesh::parse_vec3(std::ifstream & file)
{
	std::string val;
	vec3 vec;

	file >> val;
	vec.x = std::stof(val);
	file >> val;
	vec.y = std::stof(val);
	file >> val;
	vec.z = std::stof(val);
	return vec;
}

void Mesh::parse_face(std::vector<vec2>& temp_uvs, std::vector<vec3>& temp_normals, std::vector<vec3>& temp_vertices, std::vector<vec2>& in_uvs, std::vector<vec3>& in_normals, std::vector<vec3>& in_vertices, std::ifstream & file)
{
	std::string faceLine;
	std::getline(file, faceLine);
	int vertexIndex[3], uvIndex[3], normalIndex[3];

	auto parse_chunk = [](std::string str, int* v1, int* v2, int* v3)
	{
		size_t first = str.find('/');
		size_t second = str.substr(first + 1).find('/') + first + 1;

		*v1 = std::stoi(str.substr(0, first));

		if (second - first > 1)
			*v2 = std::stoi(str.substr(first + 1, second - first));
		else
			*v2 = 0;

		if (second < str.size() - 1)
			*v3 = std::stoi(str.substr(second + 1));
		else
			*v3 = 0;
	};

	size_t first = faceLine.find(' ');
	while (faceLine[first + 1] == ' ') first++;
	size_t second = faceLine.substr(first + 1).find(' ') + first + 1;
	while (faceLine[second + 1] == ' ') second++;
	size_t third = faceLine.substr(second + 1).find(' ') + second + 1;
	while (faceLine[third + 1] == ' ') third++;
	size_t last = faceLine.substr(third + 1).find(' ');

	parse_chunk(faceLine.substr(first + 1, second - 1),
		&vertexIndex[0], &uvIndex[0], &normalIndex[0]);
	parse_chunk(faceLine.substr(second + 1, third - second - 1),
		&vertexIndex[1], &uvIndex[1], &normalIndex[1]);
	parse_chunk(faceLine.substr(third + 1, last),
		&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

	vertexIndex[0]--;
	vertexIndex[1]--;
	vertexIndex[2]--;
	normalIndex[0]--;
	normalIndex[1]--;
	normalIndex[2]--;
	uvIndex[0]--;
	uvIndex[1]--;
	uvIndex[2]--;

	if (vertexIndex[0] >= 0)
	{
		in_vertices.push_back(temp_vertices[vertexIndex[0]]);
		in_vertices.push_back(temp_vertices[vertexIndex[1]]);
		in_vertices.push_back(temp_vertices[vertexIndex[2]]);
	}
	else
	{
		in_vertices.push_back(vec3{});
		in_vertices.push_back(vec3{});
		in_vertices.push_back(vec3{});
	}

	if (normalIndex[0] >= 0)
	{
		in_normals.push_back(temp_normals[normalIndex[0]]);
		in_normals.push_back(temp_normals[normalIndex[1]]);
		in_normals.push_back(temp_normals[normalIndex[2]]);
	}
	else
	{
		in_normals.push_back(vec3{});
		in_normals.push_back(vec3{});
		in_normals.push_back(vec3{});
	}

	if (uvIndex[0] >= 0)
	{
		in_uvs.push_back(temp_uvs[uvIndex[0]]);
		in_uvs.push_back(temp_uvs[uvIndex[1]]);
		in_uvs.push_back(temp_uvs[uvIndex[2]]);
	}
	else
	{
		in_uvs.push_back(vec2{});
		in_uvs.push_back(vec2{});
		in_uvs.push_back(vec2{});
	}
}

void Mesh::fill_idx_buffers(std::vector<vec3>& in_vertices, std::vector<vec2>& in_uvs, std::vector<vec3>& in_normals)
{
	struct Vertex
	{
		vec3 m_position;
		vec2 m_uv;
		vec3 m_normal;
		bool operator<(const Vertex rhs) const
		{
			return std::memcmp((void*)this, (void*)&rhs, sizeof(Vertex)) > 0;
		}
	};
	std::map<Vertex, unsigned short> VertexToOutIndex;

	for (unsigned int i = 0; i < in_vertices.size(); i++)
	{
		Vertex tmp = { in_vertices[i], in_uvs[i], in_normals[i] };
		std::map<Vertex, unsigned short>::iterator it = VertexToOutIndex.find(tmp);
		if (it == VertexToOutIndex.end())
		{
			m_vertices.push_back(in_vertices[i]);
			m_uvs.push_back(in_uvs[i]);
			m_normals.push_back(in_normals[i]);

			unsigned short newindex = (unsigned short)m_vertices.size() - 1;
			m_index.push_back(newindex);
			VertexToOutIndex[tmp] = newindex;
		}
		else
			m_index.push_back(it->second);
	}
}