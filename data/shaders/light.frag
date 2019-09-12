#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

uniform sampler2D diffuse_txt;
uniform sampler2D position_txt;
uniform sampler2D normal_txt;
uniform vec3 light_position;
uniform vec3 la;
uniform vec3 ld;
uniform vec3 ls;
uniform mat4 V;

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

	vec3 light_pos = vec3(V * vec4(light_position, 1.0));
	vec3 light_v = normalize(light_pos - frag_pos);
	vec3 view_v = normalize(-frag_pos);
	vec3 half_v = normalize(view_v + light_v);

	vec3 final_ambient = la * ka;
	vec3 final_diffuse = ld * kd * max(dot(normal_v, light_v), 0.0);
	vec3 final_specular= ls * ks * pow(max(dot(normal_v, half_v), 0.0),ns);

	out_color = vec4(final_ambient + final_diffuse + final_specular, 1.0);
}