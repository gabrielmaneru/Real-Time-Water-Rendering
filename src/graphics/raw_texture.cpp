/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	raw_texture.cpp
Purpose: Basic texture handler
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#include "raw_texture.h"
#include "gl_error.h"

void gen_texture(GLuint& id)
{
	GL_CALL(glGenTextures(1, &id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
}

void raw_texture_rgb::load()
{
	if (m_id == 0)
		gen_texture(m_id);

	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGB, GL_FLOAT, m_values.data()));
}
raw_texture_rgb& raw_texture_rgb::operator=(const map2d<vec3>& map)
{
	m_width = map.m_width;
	m_height = map.m_height;
	m_values = map.m_values;
	return *this;
}
void raw_texture_single::load()
{
	if (m_id == 0)
		gen_texture(m_id);

	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RED, GL_FLOAT, m_values.data()));
}

raw_texture_single& raw_texture_single::operator=(const map2d<float>& map)
{
	m_width = map.m_width;
	m_height = map.m_height;
	m_values = map.m_values;
	return *this;
}
