#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

layout (location = 0) uniform sampler2D diff_txt;
layout (location = 1) uniform sampler2D spec_txt;
layout (location = 2) uniform sampler2D norm_txt;
uniform bool diff_txt_active;
uniform bool spec_txt_active;
uniform bool norm_txt_active;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;

layout (location = 0) out vec4 attr_diffuse;
layout (location = 1) out vec3 attr_diffuse_rgb;
layout (location = 2) out vec4 attr_position;
layout (location = 3) out vec3 attr_position_rgb;
layout (location = 4) out vec4 attr_normal;
layout (location = 5) out vec3 attr_normal_rgb;

void main()
{
	vec3 diffuse;
	if(diff_txt_active)
	{
		vec4 txt = texture(diff_txt, vUv);
		if(txt.a < 0.5)
			discard;
		diffuse = kd * txt.rgb;
	}
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
		normal = normalize(TBN * (2.0 * texture(norm_txt, vUv).xyz - 1.0));
	}
	else
		normal = normalize(vNormal);

	attr_diffuse = vec4(diffuse, specular);
	attr_diffuse_rgb = diffuse;
	attr_position = vec4(vPosition, ka.r);
	attr_position_rgb = vPosition;
	attr_normal = vec4(normal, ns);
	attr_normal_rgb = normal;
}