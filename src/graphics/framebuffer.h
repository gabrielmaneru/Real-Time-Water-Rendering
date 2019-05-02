#pragma once

typedef unsigned int GLuint;
typedef int GLsizei;
struct framebuffer
{
	void setup(GLsizei width, GLsizei height);

	GLsizei m_width;
	GLsizei m_height;
	GLuint m_fbo;
	GLuint m_color_texture;
	GLuint m_depth_texture;
};