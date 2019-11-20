#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D depth_txt;
layout (binding = 1) uniform sampler2D noise_txt;

uniform vec2 res;
uniform vec2 inv_res;
uniform float fov;
uniform mat4 proj_mtx;

out float out_color;

const float PI = 3.1415926535897932384626433832795;

uniform vec2 random_offset;
uniform float radius;
uniform float bias;
uniform int num_dirs;
uniform int num_steps;
uniform float att;
uniform float contrast;

vec3 get_view_pos(vec2 uv)
{
	float d = texture(depth_txt, uv).x;
	vec4 view_point = vec4(uv * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
	view_point = inverse(proj_mtx) * view_point;
	view_point /= view_point.w;
	return view_point.xyz;
}
vec3 get_tangent(vec3 v_pos)
{
	vec3 v_pos_l = get_view_pos(vUv+vec2(-inv_res.x, 0));
	vec3 v_pos_r = get_view_pos(vUv+vec2( inv_res.x, 0));
	vec3 V0 = v_pos-v_pos_l;
	vec3 V1 = v_pos_r-v_pos;
	return dot(V0,V0) < dot(V1,V1) ? V0 : V1;
}

vec3 get_bitangent(vec3 v_pos)
{
	vec3 v_pos_b = get_view_pos(vUv+vec2( 0,-inv_res.y));
	vec3 v_pos_t = get_view_pos(vUv+vec2( 0, inv_res.y));
	vec3 V0 = v_pos-v_pos_b;
	vec3 V1 = v_pos_t-v_pos;
	return dot(V0,V0) < dot(V1,V1) ? V0 : V1;
}

float horizon_occlusion(vec2 uv_step, vec3 v_pos, vec3 v_tan, vec3 v_bit, float random)
{
	// Get tangent vector
	vec3 march_tan = uv_step.x * v_tan + uv_step.y * v_bit;

	// Get tangent of the angle on the Z-axis
	float march_height = march_tan.z / length(march_tan.xy) + bias;

	// Get Sinus of the angle from its tangent
	float march_sin = march_height / sqrt(march_height*march_height + 1.0);

	// Displace the starting point with some random value
	vec2 uv = vUv + random * uv_step;

	// Iterate
	float ao = 0.0;
	for(int i = 0; i < num_steps; i++)
	{
		// Increase Step
		uv += uv_step;

		// Sample position
		vec3 s_pos = get_view_pos(uv);

		// Delta vector
		vec3 d_pos = s_pos - v_pos;

		// Get tangent of the angle on the Z-axis
		float sample_height = d_pos.z / length(d_pos.xy);

		float len = length(d_pos);

		// If height is greater than the current occluded height
		if(len < radius && sample_height > march_height)
		{
			// Get Sinus of the angle from the Tangent
			float sample_sin = sample_height / sqrt(sample_height*sample_height + 1.0);

			// Occlusion step
			float local_ao = sample_sin - march_sin;
			float falloff = 1-(len*len)/(radius*radius);
			ao += local_ao * falloff * att;

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
	vec3 v_pos = get_view_pos(vUv);
	vec3 v_tan = get_tangent(v_pos);
	vec3 v_bit = get_bitangent(v_pos) * (res.y*inv_res.x);
	vec2 rand_mod_uvs = mod(vUv+random_offset, vec2(1));
	float random_value = texture2D(noise_txt, rand_mod_uvs).r;

	// Compute Radius in pixel coordinates
	float focal = 1.0f/tan(radians(fov)*0.5f);
	vec2 focal_vec = vec2(focal*res.y*inv_res.x*0.25, focal);
	float proj_radius = 0.5 * radius * focal / -v_pos.z;
	float pix_radius = proj_radius.x * res.x;


	float ao = 0.0f;
	if(pix_radius > 1.0)
	{
		// Clamp the number of steps if the pix_radius is not enought
		float step_count = min(num_steps, pix_radius);

		// Compute the step size in pixel coordinates
		float pix_step_size = pix_radius / (step_count+1);

		// Compute the step size in uv coordinates
		vec2 uv_step_size = pix_step_size * inv_res;

		// Compute the marching angle step
		float march_angle_step = 2 * PI / num_dirs;

		// Iterate directions
		for(int i = 0; i < num_dirs; i++)
		{
			// Compute the current angle using random values
			float march_angle = march_angle_step * (i + random_value);

			// Compute the marching direction from the angle
			vec2 march_dir = vec2(cos(march_angle),sin(march_angle));

			// Copmute the marching step in uv coordinates
			vec2 uv_dir_step = march_dir * uv_step_size;

			// Compute visibility factor in that direction
			ao += horizon_occlusion(uv_dir_step, v_pos, v_tan, v_bit, random_value);
		}
	}
	ao = clamp(ao*contrast, 0,1);
	out_color = 1-ao;
}