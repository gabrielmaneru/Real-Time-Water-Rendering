#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec2 attr_uvs;
layout (location = 2) in vec3 attr_norm;

uniform mat3 Vnorm;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 mPosition;
out vec3 vPosition;
out vec3 mNormal;
out vec3 vNormal;
out vec2 vUv;

void main()
{
	mNormal = attr_norm;
	vNormal = normalize(Vnorm * attr_norm);
	mPosition = attr_pos;
	vPosition = (V*vec4(attr_pos, 1.0)).xyz;
	vUv = attr_uvs;

	gl_Position = P*V*M*vec4(attr_pos, 1.0);
}