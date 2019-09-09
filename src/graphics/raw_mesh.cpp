#include "raw_mesh.h"
#include "gl_error.h"
#include <GL/gl3w.h>
void raw_mesh::load()
{
	if (m_vao == 0)
	{
		GL_CALL(glGenVertexArrays(1, &m_vao));
		GL_CALL(glBindVertexArray(m_vao));

		GL_CALL(glGenBuffers(1, &m_vtx));
		GL_CALL(glGenBuffers(1, &m_idx));
		GL_CALL(glGenBuffers(1, &m_uvs));
		GL_CALL(glGenBuffers(1, &m_norm));
	}

	GL_CALL(glBindVertexArray(m_vao));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vtx));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_DYNAMIC_DRAW));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
	GL_CALL(glEnableVertexAttribArray(0));

	int attr_off = 1;
	if (uv_coord.size())
	{
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_uvs));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, uv_coord.size() * sizeof(vec2), &uv_coord[0], GL_DYNAMIC_DRAW));
		GL_CALL(glVertexAttribPointer(attr_off, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
		GL_CALL(glEnableVertexAttribArray(attr_off++));
	}

	if (normals.size())
	{
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_norm));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_DYNAMIC_DRAW));
		GL_CALL(glVertexAttribPointer(attr_off, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
		GL_CALL(glEnableVertexAttribArray(attr_off++));
	}

	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idx));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned), &faces[0], GL_DYNAMIC_DRAW));
	
	GL_CALL(glBindVertexArray(0));
}

void raw_mesh::free()
{
	if (m_vao > 0)
	{
		GL_CALL(glDeleteBuffers(1, &m_vtx));
		GL_CALL(glDeleteBuffers(1, &m_idx));
		if(m_uvs != 0)
			GL_CALL(glDeleteBuffers(1, &m_uvs));
		if (m_norm != 0)
			GL_CALL(glDeleteBuffers(1, &m_norm));
		GL_CALL(glDeleteVertexArrays(1, &m_vao));
	}
}

void raw_mesh::compute_terrain_normals()
{
	size_t scale = static_cast<size_t>(sqrt(vertices.size()));
	normals.resize(vertices.size());
	for (size_t y = 0; y < scale; y++)
	{
		for (size_t x = 0; x < scale; x++)
		{
			size_t x_prev = (x==0)		? x : x - 1;
			size_t x_post = (x==scale-1)? x : x + 1;
			size_t y_prev = (y==0)		? y : y - 1;
			size_t y_post = (y==scale-1)? y : y + 1;

			vec3 dx = vertices[y*scale + x_post] - vertices[y*scale + x_prev];
			dx /= static_cast<float>(x_post - x_prev);
			vec3 dy = vertices[y_post*scale + x] - vertices[y_prev*scale + x];
			dy /= static_cast<float>(y_post - y_prev);
			
			normals[y*scale + x] = glm::normalize(glm::cross(dx, dy));
		}
	}
}
