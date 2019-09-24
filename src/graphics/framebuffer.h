#pragma once
#include <vector>
#include <GL/gl3w.h>
struct framebuffer
{
	void setup(GLsizei width, GLsizei height, std::vector<GLint> textures, GLuint depth = 0);

	GLsizei m_width;
	GLsizei m_height;
	GLuint m_fbo;
	std::vector<GLuint> m_color_texture;
	GLuint m_depth_texture;
};