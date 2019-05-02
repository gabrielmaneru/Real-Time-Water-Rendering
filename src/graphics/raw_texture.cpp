#include "raw_texture.h"
#include "gl_error.h"

void raw_texture::setup(size_t width, size_t height)
{
	m_width = width;
	m_height = height;
	m_pixels.reserve(m_width * m_height);
}

void raw_texture::clear(vec3 clear_color)
{
	m_pixels.resize(m_width * m_height, clear_color);
}

void raw_texture::load()
{
	if (m_id == 0)
	{
		GL_CALL(glGenTextures(1, &m_id));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	}
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGB, GL_FLOAT, m_pixels.data()));
}

vec3 raw_texture::get(size_t x, size_t y) const
{
	assert(x >= 0 && y >= 0 && x < m_width && y < m_height);
	return m_pixels[y * m_width + x];
}

void raw_texture::set(size_t x, size_t y, vec3 value)
{
	assert(x >= 0 && y >= 0 && x < m_width && y < m_height);
	m_pixels[y * m_width + x] = value;
}
