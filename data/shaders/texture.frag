#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D uniform_texture;

out vec4 out_color;

void main()
{
	vec4 texel = texture2D(uniform_texture, vUv);
	if(texel == vec4(0,0,0,0))
		discard;
	out_color = vec4(texel.rgb, 1.0);
}