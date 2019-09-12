#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

uniform sampler2D diff_txt;
uniform sampler2D spec_txt;
uniform sampler2D norm_txt;
uniform bool diff_txt_active;
uniform bool spec_txt_active;
uniform bool norm_txt_active;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;

layout (location = 0) out vec4 attr_diffuse;
layout (location = 1) out vec4 attr_position;
layout (location = 2) out vec4 attr_normal;

void main()
{
	vec3 diffuse;
	if(diff_txt_active)
		diffuse = kd * texture(diff_txt, vUv).rgb;
	else
		diffuse = kd;
	
	float specular;
	if(spec_txt_active)
		specular = ks.r * texture(spec_txt, vUv).r;
	else
		specular = ks.r;

	vec3 normal;
	if(norm_txt_active)
	{
		mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
		normal = TBN * (2.0 * texture(norm_txt, vUv).xyz - 1.0);
	}
	else
		normal = normalize(vNormal);
		
	attr_diffuse = vec4(diffuse, 1+specular);
	attr_position = vec4(vPosition, 1+ka.r);
	attr_normal = vec4(normal, 1+ns);
}