#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

layout (location = 0) uniform sampler2D diffuse_txt;
layout (location = 1) uniform sampler2D position_txt;
layout (location = 2) uniform sampler2D normal_txt;
uniform vec3 l_pos;
uniform vec3 la;
uniform vec3 ld;
uniform vec3 ls;
uniform vec3 att_factor;

layout (location = 0) out vec4 out_color;

void main()
{
	vec4 diffuse_value = texture(diffuse_txt, vUv);
	vec4 position_value = texture(position_txt, vUv);
	vec4 normal_value = texture(normal_txt, vUv);

	vec3 kd = diffuse_value.rgb;
	float ks = diffuse_value.a;
	vec3 frag_pos = position_value.rgb;
	float ka = position_value.a;
	vec3 normal_v = normalize(normal_value.rgb);
	float ns = normal_value.a;

	vec3 light_v = normalize(l_pos - frag_pos);
	vec3 view_v = normalize(-frag_pos);
	vec3 half_v = normalize(view_v + light_v);

	float d = length(l_pos - frag_pos);
	float att = min(1/(att_factor.x + att_factor.y*d + att_factor.z*d*d) , 1.0);

	float id = att * max(dot(normal_v, light_v), 0.0);
	float is = att * pow(max(dot(normal_v, half_v), 0.0),ns);

	out_color = vec4((id*ld+ka*la)*kd + ls * ks * is, 1.0);
}