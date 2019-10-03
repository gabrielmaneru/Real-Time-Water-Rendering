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
uniform vec3 selection_color;

layout (location = 0) out vec4 attr_selection;
layout (location = 1) out vec4 attr_diffuse;
layout (location = 2) out vec4 attr_position;
layout (location = 3) out vec4 attr_normal;

void main()
{
	vec3 diffuse;
	if(diff_txt_active)
	{
		vec4 txt = texture(diff_txt, vUv);
		if(txt.a < 0.5)
			discard;
		diffuse = txt.rgb;
	}
	else
		diffuse = kd;
	
	float specular;
	if(spec_txt_active)
		specular = texture(spec_txt, vUv).r;
	else
		specular = ks.r;

	vec3 normal;
	if(norm_txt_active)
	{
		const vec3 T = normalize(vTangent);
		const vec3 B = normalize(vBitangent);
		const vec3 N = normalize(vNormal);

		mat3 TBN = mat3(T,B,N);
		normal = normalize(TBN * (2.0 * texture(norm_txt, vUv).xyz - 1.0));
	}
	else
		normal = normalize(vNormal);

	attr_diffuse = vec4(diffuse, 1.0+specular);
	attr_position = vec4(vPosition, 1.0+ka.r);
	attr_normal = vec4(normal, 1.0+ns);
	attr_selection = vec4(selection_color, 1.0);
}