#include "camera.h"
#include <graphics/shader_program.h>

void camera::set_uniforms(Shader_Program * shader)
{
	shader->set_uniform("P", m_proj);
	shader->set_uniform("V", m_view);
	shader->set_uniform("M", mat4(1.0f));
}
