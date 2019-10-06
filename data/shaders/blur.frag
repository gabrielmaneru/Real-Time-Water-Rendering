#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in vec2 vUv;

layout (binding = 0) uniform sampler2D texture1;
layout (binding = 1) uniform sampler2D texture2;
layout (binding = 2) uniform sampler2D texture3;
subroutine void Execution_Type();
subroutine uniform Execution_Type execution;

layout (location = 0) out vec3 blur_factor;
layout (location = 1) out vec3 final_color;

//--Sobel-Edge-Detection---------------------------------------------
uniform float width;
uniform float height;
uniform float coef_normal;
uniform float coef_depth;
uniform float depth_power;
uniform int sigma=1;

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
	sobel_d = 1.0f-pow(1.0f-sobel_d,50);	

	blur_factor.x = clamp(sobel_n * coef_normal + sobel_d * coef_depth, 0.0, 1.0);
}
//-------------------------------------------------------------------



//--Gaussian-blur----------------------------------------------------
const float gaussian[][]= float[][](
	float[](0.000036,0.000363,0.001446,0.002291,0.003676,0.014662,0.023226,0.058488,0.092651,0.146768),//sigma=1
	float[](0.005084,0.009377,0.013539,0.015302,0.017296,0.024972,0.028224,0.036054,0.040749,0.046056),//sigma=2
	float[](0.011362,0.014962,0.017649,0.018648,0.019703,0.023240,0.024556,0.027413,0.028964,0.030603),//sigma=3
	float[](0.014786,0.017272,0.018961,0.019559,0.020177,0.022149,0.022849,0.024314,0.025082,0.025874),//sigma=4
	float[](0.016641,0.018385,0.019518,0.019911,0.020312,0.021564,0.021998,0.022893,0.023354,0.023824)//sigma=5
	);
subroutine (Execution_Type)
void do_gaussian_blur()
{
	float w = 1.0/width;
	float h = 1.0/height;
	
	vec3 sum=vec3(0,0,0);
	int idx = sigma-1;
	sum+=gaussian[idx][0]*texture2D(texture1, vUv + vec2(-3*w,	-3*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2(-2*w,	-3*h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2(-w,	-3*h)).rgb;
	sum+=gaussian[idx][3]*texture2D(texture1, vUv + vec2(0.0,	-3*h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2( w,	-3*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2( 2*w,	-3*h)).rgb;
	sum+=gaussian[idx][0]*texture2D(texture1, vUv + vec2( 3*w,	-3*h)).rgb;

	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2(-3*w,	-2*h)).rgb;
	sum+=gaussian[idx][4]*texture2D(texture1, vUv + vec2(-2*w,	-2*h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2(-w,	-2*h)).rgb;
	sum+=gaussian[idx][6]*texture2D(texture1, vUv + vec2(0.0,	-2*h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2( w,	-2*h)).rgb;
	sum+=gaussian[idx][4]*texture2D(texture1, vUv + vec2( 2*w,	-2*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2( 3*w,	-2*h)).rgb;

	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2(-3*w,	-h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2(-2*w,	-h)).rgb;
	sum+=gaussian[idx][7]*texture2D(texture1, vUv + vec2(-w,	-h)).rgb;
	sum+=gaussian[idx][8]*texture2D(texture1, vUv + vec2(0.0,	-h)).rgb;
	sum+=gaussian[idx][7]*texture2D(texture1, vUv + vec2( w,	-h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2( 2*w,	-h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2( 3*w,	-h)).rgb;

	sum+=gaussian[idx][3]*texture2D(texture1, vUv + vec2(-3*w,	0.0)).rgb;
	sum+=gaussian[idx][6]*texture2D(texture1, vUv + vec2(-2*w,	0.0)).rgb;
	sum+=gaussian[idx][8]*texture2D(texture1, vUv + vec2(-w,	0.0)).rgb;
	sum+=gaussian[idx][9]*texture2D(texture1, vUv + vec2(0.0,	0.0)).rgb;
	sum+=gaussian[idx][8]*texture2D(texture1, vUv + vec2( w,	0.0)).rgb;
	sum+=gaussian[idx][6]*texture2D(texture1, vUv + vec2( 2*w,	0.0)).rgb;
	sum+=gaussian[idx][3]*texture2D(texture1, vUv + vec2( 3*w,	0.0)).rgb;

	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2(-3*w,	h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2(-2*w,	h)).rgb;
	sum+=gaussian[idx][7]*texture2D(texture1, vUv + vec2(-w,	h)).rgb;
	sum+=gaussian[idx][8]*texture2D(texture1, vUv + vec2(0.0,	h)).rgb;
	sum+=gaussian[idx][7]*texture2D(texture1, vUv + vec2( w,	h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2( 2*w,	h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2( 3*w,	h)).rgb;

	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2(-3*w,	2*h)).rgb;
	sum+=gaussian[idx][4]*texture2D(texture1, vUv + vec2(-2*w,	2*h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2(-w,	2*h)).rgb;
	sum+=gaussian[idx][6]*texture2D(texture1, vUv + vec2(0.0,	2*h)).rgb;
	sum+=gaussian[idx][5]*texture2D(texture1, vUv + vec2( w,	2*h)).rgb;
	sum+=gaussian[idx][4]*texture2D(texture1, vUv + vec2( 2*w,	2*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2( 3*w,	2*h)).rgb;

	sum+=gaussian[idx][0]*texture2D(texture1, vUv + vec2(-3*w,	3*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2(-2*w,	3*h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2(-w,	3*h)).rgb;
	sum+=gaussian[idx][3]*texture2D(texture1, vUv + vec2(0.0,	3*h)).rgb;
	sum+=gaussian[idx][2]*texture2D(texture1, vUv + vec2( w,	3*h)).rgb;
	sum+=gaussian[idx][1]*texture2D(texture1, vUv + vec2( 2*w,	3*h)).rgb;
	sum+=gaussian[idx][0]*texture2D(texture1, vUv + vec2( 3*w,	3*h)).rgb;

	final_color = sum;
}
//-------------------------------------------------------------------



//----Final-blur-----------------------------------------------------
subroutine (Execution_Type)
void do_final_blur()
{
	blur_factor = mix(texture2D(texture3, vUv).rgb,texture2D(texture2, vUv).rgb,texture2D(texture1, vUv).r);
}
//-------------------------------------------------------------------

void main()
{
	execution();
}