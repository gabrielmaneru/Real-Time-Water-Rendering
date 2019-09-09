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
void raw_texture_single::load()
{
	if (m_id == 0)
		gen_texture(m_id);

	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RED, GL_FLOAT, m_values.data()));
}
