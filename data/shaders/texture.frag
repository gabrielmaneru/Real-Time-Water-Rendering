#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

layout (location = 0) uniform sampler2D uniform_texture;

out vec4 out_color;

void main()
{
	out_color = vec4(texture2D(uniform_texture, vUv).rgb, 1.0);
}