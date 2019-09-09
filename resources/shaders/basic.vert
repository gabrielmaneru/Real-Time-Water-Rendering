#version 440
in vec3 attr_pos;
in vec2 attr_uvs;
in vec3 attr_norm;

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