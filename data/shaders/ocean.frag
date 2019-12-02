#version 440

in vec3 mPosition;
in vec3 vPosition;
in vec3 mNormal;
in vec3 vNormal;
in vec2 vUv;

uniform float near;
uniform float far;
uniform mat3 Vnorm;
uniform mat4 P;
layout (binding = 0) uniform samplerCube skybox_txt;
layout (binding = 1) uniform sampler2D depth_txt;
layout (binding = 2) uniform sampler2D diffuse_txt;

uniform bool line_render;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out float attr_lindepth;

vec3 get_vpos()
{
	vec2 size = vec2(textureSize(depth_txt,0));
	vec2 txt_uvs = vec2(gl_FragCoord.x/size.x, gl_FragCoord.y/size.y);
	float depth = texture(depth_txt, txt_uvs).x;
	vec4 view_point = vec4(txt_uvs * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	view_point = inverse(P) * view_point;
	view_point /= view_point.w;
	return view_point.xyz;
}
vec4 get_prev_diff(vec2 dUv)
{
	vec2 size = vec2(textureSize(diffuse_txt,0));
	vec2 txt_uvs = vec2(gl_FragCoord.x/size.x, gl_FragCoord.y/size.y);
	return texture(diffuse_txt, txt_uvs+dUv);
}

void main()
{
	if(line_render)
	{
		attr_albedo = vec4(0.0, 0.0, 0.0, 1.0);
		attr_normal = vec4(normalize(vNormal), 1.0);
		return;
	}

	float water_len = length(vPosition-get_vpos());
	const float sea_view = 50.0f;
	float depth_factor = clamp(water_len/sea_view,0.0,1.0);
	vec3 blue_water_color = 0.5*vec3(0.00,0.47,0.75);
	vec3 green_water_color = 0.5*vec3(0.08,0.85,0.57);

	vec3 vView = normalize(vPosition);
	vec3 vReflect_View = normalize(reflect(vView, vNormal));
	vec3 wReflect_View = normalize(inverse(Vnorm)*vReflect_View);
	vec3 sky_color = texture(skybox_txt, wReflect_View).rgb;
	if(wReflect_View.y < 0) 
		sky_color = vec3(0.9);

	vec3 vRefract_View = normalize(refract(vView, vNormal, 0.75));
	float refract_factor = pow(depth_factor,0.8);
	vec2 delta_Refract = (vRefract_View.xy-vView.xy)*refract_factor;
	vec4 prev_diff = get_prev_diff(delta_Refract);
	vec3 prev_color = prev_diff.xyz;

	float drag_factor = pow(depth_factor,0.3);
	vec3 drag_color = mix(green_water_color,blue_water_color,drag_factor);
	float color_factor = pow(depth_factor,0.25);
	vec3 diffuse = mix(prev_color,drag_color,color_factor);
	
	diffuse += clamp(pow(vNormal.y, 4),0,1)*vec3(0.5*sky_color);
	//diffuse = vec3 (refract_factor);

	attr_albedo = vec4(diffuse, 1.0);
	attr_metallic = vec4(vec3(0.0), 1.0);
	attr_normal = vec4(normalize(vNormal), 1.0);
	attr_position = vec4(vPosition, 2.0);
    float d = (near * far) / (far - near + vPosition.z);
	attr_lindepth = 1.0f-pow(1.0f-d,10);
}