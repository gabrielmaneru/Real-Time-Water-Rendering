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
	vec3 normal = normalize(vNormal);

	if(diff_txt_active)
		diffuse = texture(diff_txt, vUv).rgb;
	if(spec_txt_active)
		specular *= texture(spec_txt, vUv).rgb;
	if(norm_txt_active)
	{
		normal = 2.0 * texture(norm_txt, vUv).xyz - 1.0;
		mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
		normal = TBN*normal;
	}

	vec3 view = vec3(0.0,0.0,-1.0);
	vec3 light = vec3(0.0,0.0,-1.0);
	vec3 reflect = 2*dot(normal,light)-light;

	float Ia = 0.1;
	float Id = max(dot(normal, light), 0);
	float Is = pow(max(dot(reflect, view), 0),ns);

	out_color = vec4(diffuse, 1.0) + 0.0001 *vec4(ka*Ia + Id*diffuse + Is*specular, 1.0);
}