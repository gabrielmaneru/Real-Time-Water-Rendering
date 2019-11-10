#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D position_txt;
layout (binding = 1) uniform sampler2D normal_txt;
uniform float width;
uniform float height;
uniform float random01=0.0;

out float out_color;

const float PI = 3.1415926535897932384626433832795;

uniform float radius = 2.0;
const float angle_bias = 0.1f;
uniform int num_dirs = 4;
uniform int num_steps = 4;
const float att = 0.1f;
const float contrast = 1.0f;

vec3 get_tangent()
{
	float w = 1/width;
	float h = 1/height;
	vec3 v_norm_l = texture2D(normal_txt, vUv+vec2(-w, 0)).rgb;
	vec3 v_norm_r = texture2D(normal_txt, vUv+vec2( w, 0)).rgb;
	return normalize(v_norm_r-v_norm_l);
}

vec3 get_bitangent()
{
	float w = 1/width;
	float h = 1/height;
	vec3 v_norm_b = texture2D(normal_txt, vUv+vec2( 0,-h)).rgb;
	vec3 v_norm_t = texture2D(normal_txt, vUv+vec2( 0, h)).rgb;
	return normalize(v_norm_t-v_norm_b);
}

void main()
{
	// Extract image data
	vec3 v_pos = texture2D(position_txt, vUv).rgb;
	vec3 v_tan = get_tangent();
	vec3 v_bit = get_bitangent();

	// Compute steps
	float march_angle_step = 2 * PI / num_dirs;
	float march_dist_step = radius / num_steps;

	// Iterate directions
	float ao = 0.0;
	for(int i = 0; i < num_dirs; i++)
	{
		// Compute current values
		float march_angle = march_angle_step * (i + random01);
		float cos_angle = cos(march_angle);
		float sin_angle = sin(march_angle);
		vec2 march_dir = vec2(cos_angle,sin_angle);

		// Get tangent angle
		vec3 march_tan = v_tan*cos_angle + v_bit*sin_angle;
		float tan_angle = acos(dot(normalize(march_tan), vec3(march_dir,0)));

		// Get max horizon angle
		float max_horizon_angle = 0.0;
		for(int i = 0; i < num_steps; i++)
		{
			vec2 off = march_dir * (march_dist_step*i);
			vec3 s_pos = texture2D(position_txt, vUv+off).rgb;
			vec3 delta = s_pos - v_pos;
			float angle = acos(dot(normalize(delta), vec3(march_dir,0)));
			max_horizon_angle = max(max_horizon_angle, angle);
		}
		float local_ao = sin(max_horizon_angle) - sin(tan_angle);
		ao += local_ao/num_dirs;
	}

	out_color = ao;
}