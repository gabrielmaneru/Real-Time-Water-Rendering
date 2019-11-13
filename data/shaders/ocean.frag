#version 440

in vec3 vNormal;
in vec3 mPosition;
in vec3 vPosition;
in vec2 vUv;

uniform float near;
uniform float far;

uniform vec3 color;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out float attr_lindepth;

void main()
{
	attr_position = vec4(vPosition, 1.6);
	attr_albedo = vec4(color, 1.0);
	attr_metallic = vec4(vec3(0.0), 1.0);
	attr_normal = vec4(normalize(vNormal), 1.0);

    float d = (near * far) / (far - near + vPosition.z);
	attr_lindepth = 1.0f-pow(1.0f-d,10);
}