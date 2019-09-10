#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_norm;
layout (location = 2) in vec2 attr_uvs;
layout (location = 3) in vec3 attr_tan;
layout (location = 4) in vec2 attr_bit;

uniform mat4 Model;
uniform mat4 VP;

out vec2 uv;
out vec3 normal;

void main()
{
	uv = attr_uvs;
	normal = attr_norm;

	vec4 wPos = Model * vec4(attr_pos, 1.0);
	gl_Position = VP * wPos;
}