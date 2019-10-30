/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	framebuffer.h
Purpose: Framebuffer Handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <vector>
#include <GL/gl3w.h>
struct framebuffer
{
	void setup(GLsizei width, GLsizei height, std::vector<GLint> textures, GLuint depth = 0);
	void set_drawbuffers(const std::vector<GLenum>& attachs = {});

	GLsizei m_width{0};
	GLsizei m_height{0};
	GLuint m_fbo;
	std::vector<GLuint> m_color_texture;
	GLuint m_depth_texture;
};