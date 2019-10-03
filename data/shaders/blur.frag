#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

layout (binding = 0) uniform sampler2D texture1;
layout (binding = 1) uniform sampler2D texture2;
subroutine void Execution_Type();
subroutine uniform Execution_Type execution;

layout (location = 0) out float blur_factor;
layout (location = 1) out vec3 final_color;

//--Sobel-Edge-Detection---------------------------------------------
uniform float width;
uniform float height;
uniform float coef_normal;
uniform float coef_depth;
uniform float depth_power;

subroutine (Execution_Type)
void do_sobel_edge_detection()
{
	float w = 1.0/width;
	float h = 1.0/height;
	
	vec3 n_0=texture2D(texture1, vUv + vec2(-w,-h)).rgb;
	vec3 n_1=texture2D(texture1, vUv + vec2(0.0,-h)).rgb;
	vec3 n_2=texture2D(texture1, vUv + vec2(w,-h)).rgb;
	vec3 n_3=texture2D(texture1, vUv + vec2(-w,0.0)).rgb;
	vec3 n_4=texture2D(texture1, vUv).rgb;
	vec3 n_5=texture2D(texture1, vUv + vec2(w,0.0)).rgb;
	vec3 n_6=texture2D(texture1, vUv + vec2(-w,h)).rgb;
	vec3 n_7=texture2D(texture1, vUv + vec2(0.0,h)).rgb;
	vec3 n_8=texture2D(texture1, vUv + vec2(w,h)).rgb;

	float d_0=texture2D(texture2, vUv + vec2(-w,-h)).r;
	float d_1=texture2D(texture2, vUv + vec2(0.0,-h)).r;
	float d_2=texture2D(texture2, vUv + vec2(w,-h)).r;
	float d_3=texture2D(texture2, vUv + vec2(-w,0.0)).r;
	float d_4=texture2D(texture2, vUv).r;
	float d_5=texture2D(texture2, vUv + vec2(w,0.0)).r;
	float d_6=texture2D(texture2, vUv + vec2(-w,h)).r;
	float d_7=texture2D(texture2, vUv + vec2(0.0,h)).r;
	float d_8=texture2D(texture2, vUv + vec2(w,h)).r;
	
	vec3 sobel_n_h = n_2 + 2.0*n_5 + n_8 - (n_0 + 2.0*n_3 + n_6);
	vec3 sobel_n_v = n_0 + 2.0*n_1 + n_2 - (n_6 + 2.0*n_7 + n_8);
	float sobel_n = length(sqrt(sobel_n_h * sobel_n_h + sobel_n_v * sobel_n_v));

	float sobel_d_h = d_2 + 2.0*d_5 + d_8 - (d_0 + 2.0*d_3 + d_6);
	float sobel_d_v = d_0 + 2.0*d_1 + d_2 - (d_6 + 2.0*d_7 + d_8);
	float sobel_d = sqrt(sobel_d_h * sobel_d_h + sobel_d_v * sobel_d_v);
	sobel_d = 1.0f-pow(1.0f-sobel_d,depth_power);

	blur_factor = clamp(sobel_n * coef_normal + sobel_d * coef_depth, 0.0, 1.0);
}
//-------------------------------------------------------------------


//--Sobel-Edge-Detection---------------------------------------------
subroutine (Execution_Type)
void do_blur()
{
	blur_factor = texture2D(texture1, vUv).r;
	final_color = vec3(texture2D(texture1, vUv).r);
}
//-------------------------------------------------------------------

void main()
{
	execution();
}