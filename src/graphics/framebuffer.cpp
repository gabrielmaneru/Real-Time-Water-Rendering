/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	framebuffer.cpp
Purpose: Framebuffer Handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "framebuffer.h"
#include <GL/gl3w.h>
#include "gl_error.h"

void framebuffer::setup(GLsizei width, GLsizei height, std::vector<GLint> textures, GLuint depth)
{
	m_width = width;
	m_height = height;

	GL_CALL(glGenFramebuffers(1, &m_fbo));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

	// Diffuse Textures
	m_color_texture.resize(textures.size()/4);
	for (GLenum i = 0; i < (GLenum)m_color_texture.size(); i++)
	{
		// Create Texture
		GL_CALL(glGenTextures(1, &m_color_texture[i]));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, m_color_texture[i]));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, textures[i * 4], m_width, m_height, 0, textures[i * 4 + 1], textures[i * 4 + 2], nullptr));

		// Set Parameters
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textures[i * 4 + 3]));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textures[i * 4 + 3]));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

		// Attach
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_color_texture[i], 0));
	}

	// Pass Layout
	std::vector<GLenum> draw_buffers;
	for (GLenum i = 0; i < (GLenum)m_color_texture.size(); i++)
		draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
	GL_CALL(glDrawBuffers((GLsizei)m_color_texture.size(), draw_buffers.data()));

	// Depth Texture
	{
		if (depth == 0)
		{
			// Create Texture
			GL_CALL(glGenTextures(1, &m_depth_texture));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, m_depth_texture));
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));

			// Set Parameters
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
		}
		else
			m_depth_texture = depth;

		// Attach
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_texture, 0));
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw "Invalid Framebuffer";

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
