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

out vec4 out_color;

void main()
{
	vec3 diffuse = vec3(kd);
	vec3 specular = vec3(ks);
	
	if(diff_txt_active)
		diffuse *= texture(diff_txt, vUv).rgb;
	if(spec_txt_active)
		specular *= texture(spec_txt, vUv).rgb;

	out_color = vec4(diffuse, 1.0);
}