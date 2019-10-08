#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

uniform vec3 color;

out vec4 out_color;

void main()
{
	out_color = vec4(color, 1.0+vMotion);
}