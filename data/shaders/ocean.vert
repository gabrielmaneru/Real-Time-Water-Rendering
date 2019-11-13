#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 2) in vec2 attr_uvs;
layout (location = 1) in vec3 attr_norm;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 vNormal;
out vec3 mPosition;
out vec3 vPosition;
out vec2 vUv;

void main()
{
	mat4 MV = V*M;
	mat3 normalMtx = inverse(transpose(mat3(MV)));

	vNormal = normalize(normalMtx * attr_norm);
	mPosition = attr_pos;
	vPosition = (MV*vec4(attr_pos, 1.0)).xyz;
	vUv = attr_uvs;

	gl_Position = P*MV* vec4(attr_pos, 1.0);
}