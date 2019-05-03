#version 440
in vec3 attr_pos;
in vec2 attr_uv;
uniform mat4 MVP;

out vec2 var_uv;

void main()
{
	vec4 vertex = vec4(attr_pos, 1.0f);
	var_uv = attr_uv;
	gl_Position = MVP * vertex;
}