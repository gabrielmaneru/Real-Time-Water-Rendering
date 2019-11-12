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

void main()
{
	vec2 new_uvs = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
	
	if(length(texture(normal_txt, new_uvs).xyz) > 0)
		discard;

	vec3 cube_uvs = normalize(mPosition);
	out_color = texture(skybox_txt, cube_uvs).rgb;
}