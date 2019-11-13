#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 mPosition;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D normal_txt;
layout (binding = 1) uniform samplerCube skybox_txt;
uniform float width;
uniform float height;

out vec3 out_color;

const float PI = 3.14159;

float map(float x, float xmin, float xmax, float ymin, float ymax)
{
	float c = (x-xmin)/(xmax-xmin);
	return mix(ymin,ymax,c);
}

void main()
{
	vec2 new_uvs = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
	
	if(length(texture(normal_txt, new_uvs).xyz) > 0)
		discard;

	vec3 n_pos = normalize(mPosition);
	out_color = texture(skybox_txt, n_pos).rgb;
}