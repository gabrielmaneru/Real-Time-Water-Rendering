#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D texture1;
layout (binding = 1) uniform sampler2D texture2;
layout (binding = 2) uniform sampler2D texture3;
subroutine void Execution_Type();
subroutine uniform Execution_Type execution;

layout (location = 0) out vec3 attr_1;
layout (location = 1) out vec3 attr_2;

uniform int pass;
uniform float width;
uniform float height;
uniform int blur_mode;
uniform float bilat_scale;
const float gaussian[]= float[](0.129001,0.142521,0.151303,0.15435);
vec3 get_color(vec2 off)
{
	return clamp(texture2D(texture1, vUv + off).rgb,vec3(0,0,0),vec3(1,1,1));
}
float bilateral(vec2 uv)
{
	float center = texture2D(texture3, vUv).r;
	return 1/(1+bilat_scale*abs(texture2D(texture3, vUv+uv).r - center));
}
vec3 blur_3()
{
	float w = 1.0/width;
	float h = 1.0/height;
	
	vec3 sum=vec3(0.0);
	if(blur_mode == 0)
	{
		if(pass == 0)
		{
			sum += gaussian[0] * get_color(vec2(-3*w,   0));
			sum += gaussian[1] * get_color(vec2(-2*w,   0));
			sum += gaussian[2] * get_color(vec2(  -w,   0));
			sum += gaussian[3] * get_color(vec2(   0,   0));
			sum += gaussian[2] * get_color(vec2(   w,   0));
			sum += gaussian[1] * get_color(vec2( 2*w,   0));
			sum += gaussian[0] * get_color(vec2( 3*w,   0));
		}
		else
		{
			sum += gaussian[0] * get_color(vec2(   0,-3*w));
			sum += gaussian[1] * get_color(vec2(   0,-2*w));
			sum += gaussian[2] * get_color(vec2(   0,  -w));
			sum += gaussian[3] * get_color(vec2(   0,   0));
			sum += gaussian[2] * get_color(vec2(   0,   w));
			sum += gaussian[1] * get_color(vec2(   0, 2*w));
			sum += gaussian[0] * get_color(vec2(   0, 3*w));
		}
	}
	else
	{
		float tot_bil = 0.0f;
		if(pass == 0)
		{
			tot_bil += bilateral(vec2(-3*w,0));
			sum += gaussian[0] * bilateral(vec2(-3*w,0)) * get_color(vec2(-3*w,0));
			tot_bil += bilateral(vec2(-2*w,0));
			sum += gaussian[1] * bilateral(vec2(-2*w,0)) * get_color(vec2(-2*w,0));
			tot_bil += bilateral(vec2(-w,0));
			sum += gaussian[2] * bilateral(vec2(-w,0)) * get_color(vec2(-w,0));
			tot_bil += bilateral(vec2(0,0));
			sum += gaussian[3] * bilateral(vec2(0,0)) * get_color(vec2(0,0));
			tot_bil += bilateral(vec2(w,0));
			sum += gaussian[2] * bilateral(vec2(w,0)) * get_color(vec2(w,0));
			tot_bil += bilateral(vec2(2*w,0));
			sum += gaussian[1] * bilateral(vec2(2*w,0)) * get_color(vec2(2*w,0));
			tot_bil += bilateral(vec2(3*w,0));
			sum += gaussian[0] * bilateral(vec2(3*w,0)) * get_color(vec2(3*w,0));
			sum = vec3(tot_bil);
		}
		else
		{
			tot_bil += bilateral(vec2(0,-3*w));
			sum += gaussian[0] * bilateral(vec2(0,-3*w)) * get_color(vec2(0,-3*w));
			tot_bil += bilateral(vec2(0,-2*w));
			sum += gaussian[1] * bilateral(vec2(0,-2*w)) * get_color(vec2(0,-2*w));
			tot_bil += bilateral(vec2(0,-w));
			sum += gaussian[2] * bilateral(vec2(0,-w)) * get_color(vec2(0,-w));
			tot_bil += bilateral(vec2(0,0));
			sum += gaussian[3] * bilateral(vec2(0,0)) * get_color(vec2(0,0));
			tot_bil += bilateral(vec2(0,w));
			sum += gaussian[2] * bilateral(vec2(0,w)) * get_color(vec2(0,w));
			tot_bil += bilateral(vec2(0,2*w));
			sum += gaussian[1] * bilateral(vec2(0,2*w)) * get_color(vec2(0,2*w));
			tot_bil += bilateral(vec2(0,3*w));
			sum += gaussian[0] * bilateral(vec2(0,3*w)) * get_color(vec2(0,3*w));
			sum = vec3(tot_bil);
		}
	}
	return sum;
}


//--Sobel-Edge-Detection---------------------------------------------
uniform float coef_normal;
uniform float coef_depth;

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
	sobel_d = 1.0f-pow(1.0f-sobel_d,20);	

	attr_1.r = clamp(sobel_n * coef_normal + sobel_d * coef_depth, 0.0, 1.0);
}
//-------------------------------------------------------------------



//--Depth-of-Field---------------------------------------------------
uniform float focal_distance;
uniform float aperture;

subroutine (Execution_Type)
void do_depth_of_field()
{
	vec3 frag_pos=texture2D(texture1, vUv).rgb;
	float z_dist = -frag_pos.z;
	float diff = abs(z_dist - focal_distance)/aperture;

	attr_1.g=diff;
}
//-------------------------------------------------------------------



//--Motion-Blur------------------------------------------------------
subroutine (Execution_Type)
void do_motion_blur()
{
	attr_1.b = texture2D(texture1, vUv).a-1.0f;
}
//-------------------------------------------------------------------



//--Bloom-Filter-----------------------------------------------------
uniform float bloom_coef;
subroutine (Execution_Type)
void do_bloom()
{
	vec3 frag_color=texture2D(texture1, vUv).rgb;
	float brithness = dot(frag_color, vec3(0.2126, 0.7152, 0.0722));
	
	if(brithness > bloom_coef)
		attr_1 = frag_color;
	else
		attr_1 = vec3(0.0);
}
subroutine (Execution_Type)
void do_bloom_blur()
{
	attr_1 = blur_3();
}
//-------------------------------------------------------------------



//----Final-blur-----------------------------------------------------
uniform int final_mode;
subroutine (Execution_Type)
void do_final_blur()
{

	if(final_mode == 0)
		attr_1 = texture2D(texture1, vUv).rgb;
	else if (final_mode == 1)
	{
		vec3 sum = blur_3();
		attr_1 = mix(texture2D(texture1, vUv).rgb, sum, clamp(length(texture2D(texture2, vUv).rgb),0,1));
	}
	else
		attr_1 = texture2D(texture1, vUv).rgb + texture2D(texture3, vUv).rgb;
}
//-------------------------------------------------------------------

void main()
{
	execution();
}