#version 440

in vec3 Normal_tes;
in vec3 Tangent_tes;
in vec3 Bitangent_tes;
in vec3 Position_tes;
in vec2 Uv_tes;
in float adapt_value_tes;

layout (binding = 0) uniform sampler2D albedo_txt;
layout (binding = 1) uniform sampler2D metallic_txt;
layout (binding = 2) uniform sampler2D roughness_txt;
layout (binding = 3) uniform sampler2D normal_txt;

uniform bool albedo_txt_active;
uniform bool metallic_txt_active;
uniform bool roughness_txt_active;
uniform bool normal_txt_active;

uniform vec3 kalbedo;
uniform vec3 kmetallic;
uniform float kroughness;
uniform float kambient;

uniform float near;
uniform float far;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out vec4 attr_adaptive;

vec3 hue(float x)
{
	float angle = mix(270, 0, x);
	float val = mod(angle, 60) / 60;

	vec3 color;
	if	(angle < 60)
		color = vec3( 1.0, val, 0.0 );
	
	else if (angle < 120)
		color = vec3( 1.0-val, 1.0, 0.0 );
	
	else if (angle < 180)
		color = vec3( 0.0, 1.0, val );
	
	else if (angle < 240)
		color = vec3( 0.0, 1.0-val, 1.0);
	
	else if (angle < 300)
		color = vec3( val, 0.0, 1.0 );
	
	else
		color = vec3( 1.0, 0.0, 1.0-val );
	return color;
}

void main()
{
	vec3 albedo;
	if(albedo_txt_active)
	{
		vec4 txt = texture(albedo_txt, Uv_tes);
		if(txt.a < 0.5)
			discard;
		albedo = txt.rgb;
	}
	else
		albedo = kalbedo;
	
	vec3 metallic;
	if(metallic_txt_active)
		metallic = vec3(texture(metallic_txt, Uv_tes).r);
	else
		metallic = kmetallic;
	
	float roughness;
	if(roughness_txt_active)
		roughness = texture(roughness_txt, Uv_tes).r;
	else
		roughness = kroughness;

	vec3 normal;
	if(normal_txt_active)
	{
		const vec3 T = normalize(Tangent_tes);
		const vec3 B = normalize(Bitangent_tes);
		const vec3 N = normalize(Normal_tes);

		mat3 TBN = mat3(T,B,N);
		normal = normalize(TBN * (2.0 * texture(normal_txt, Uv_tes).xyz - 1.0));
	}
	else
		normal = normalize(Normal_tes);

	attr_position = vec4(Position_tes, 1.0+kambient);
	attr_albedo = vec4(albedo, 1.0+roughness);
	attr_metallic = vec4(metallic, 1.0);
	attr_normal = vec4(normal, 1.0);

	attr_adaptive = vec4(hue(adapt_value_tes), 1.0);
	
    gl_FragDepth = (near * far) / (far - near + Position_tes.z);
	gl_FragDepth = 1.0f-pow(1.0f-gl_FragDepth,10);	
}