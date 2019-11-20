#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D position_txt;
layout (binding = 1) uniform sampler2D normal_txt;
layout (binding = 2) uniform sampler2D noise_txt;

uniform float width;
uniform float height;
uniform float fov;

out float out_color;

const float PI = 3.1415926535897932384626433832795;

uniform bool noise;
uniform vec2 random_offset;
uniform float radius = 2.0;
uniform float bias = 0.1;
uniform int num_dirs = 4;
uniform int num_steps = 4;
const float att = 1.0f;
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

float horizon_occlusion(vec2 uv_step, vec3 v_pos, vec3 v_tan, vec3 v_bit)
{
	// Get tangent vector
	vec3 march_tan = uv_step.x * v_tan + uv_step.y * v_bit;

	// Get tangent of the angle on the Z
	float march_height = march_tan.z / length(march_tan.xy) + bias;

	// Get Sin of the angle from the Tangent
	float march_sin = march_height / sqrt(march_height*march_height + 1.0);

	// Iterate
	vec2 uv = vUv;
	float ao = 0.0;
	for(int i = 0; i < num_steps; i++)
	{
		// Increase Step
		uv += uv_step;

		// Sample position
		vec3 s_pos = texture2D(position_txt, uv).rgb;

		// Delta vector
		vec3 d_pos = s_pos - v_pos;

		// Get tangent of the angle on the Z
		float sample_height = d_pos.z / length(d_pos.xy);

		float len = length(d_pos);

		// If height is greater than the current occluded height
		if(len < radius && sample_height > march_height)
		{
			// Get Sin of the angle from the Tangent
			float sample_sin = sample_height / sqrt(sample_height*sample_height + 1.0);

			// Occlusion step
			float local_ao = sample_sin - march_sin;
			ao += mix(local_ao,local_ao * pow(1-len/radius,2),att);

			// Update current occlusion
			march_height = sample_height;
			march_sin = sample_sin;
		}
	}
	return ao;
}

void main()
{
	// Extract image data
	vec3 v_pos = texture2D(position_txt, vUv).rgb;
	vec3 v_tan = get_tangent();
	vec3 v_bit = get_bitangent();
	float random_value = texture2D(noise_txt, mod(vUv+random_offset, vec2(1))).r;

	// Compute steps
	float w = 1/width;
	float h = 1/height;
	float march_angle_step = 2 * PI / num_dirs;
	float p_radius = radius / -v_pos.z;
	vec2 march_dist_step = vec2(p_radius/num_steps);

	// Iterate directions
	float ao = 0.0;
	for(int i = 0; i < num_dirs; i++)
	{
		float march_angle = march_angle_step * (i + random_value);
		vec2 march_dir = vec2(cos(march_angle),sin(march_angle));
		vec2 uv_step = march_dir * march_dist_step;

		float local_ao = horizon_occlusion(uv_step, v_pos, v_tan, v_bit);
		ao += max(local_ao, 0);
	}

	out_color = 1-ao;
	if(noise)
		out_color = random_value;
}