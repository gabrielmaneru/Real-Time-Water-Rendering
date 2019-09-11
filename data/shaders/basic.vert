#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_norm;
layout (location = 2) in vec2 attr_uvs;
layout (location = 3) in vec3 attr_tan;
layout (location = 4) in vec3 attr_bit;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vPosition;
out vec2 vUv;

void main()
{
	vNormal = attr_norm;
	vTangent = attr_tan;
	vBitangent = attr_bit;
	vPosition = (V*M*vec4(attr_pos, 1.0)).xyz;
	vUv = attr_uvs;

	gl_Position = P*V*M * vec4(attr_pos, 1.0);
}