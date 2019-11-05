#version 440

layout (binding = 0) uniform sampler2D albedo_txt;
layout (binding = 1) uniform sampler2D normal_txt;
layout (binding = 2) uniform sampler2D depth_txt;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform float width;
uniform float height;
uniform float angle;
uniform int mode;

layout (location = 0) out vec4 attr_albedo;
layout (location = 1) out vec4 attr_normal;

void main()
{
	if(mode==1)
	{
		attr_albedo = vec4(1.0);
		attr_normal = vec4(1.0);
		return;
	}

	vec2 txt_uvs = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
	float depth = texture(depth_txt, txt_uvs).x;
	vec4 view_point = vec4(txt_uvs * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	view_point = inverse(P) * view_point;
	view_point /= view_point.w;
	vec4 world_point = inverse(V) * view_point;
	vec4 model_point = inverse(M) * world_point;

	if(abs(model_point.x) > 0.5 || abs(model_point.y) > 0.5 || abs(model_point.z) > 0.5)
		discard;

	if(mode==2)
	{
		attr_albedo = vec4(1.0);
		attr_normal = vec4(1.0);
		return;
	}

	vec2 model_uv = model_point.xy+vec2(0.5);
	vec4 txt = texture(albedo_txt, model_uv);
	if(txt.a < 0.5)
		discard;
	
	vec3 world_t = normalize(dFdx(world_point.xyz));
	vec3 world_b = normalize(dFdy(world_point.xyz));
	vec3 world_n = cross(world_t,world_b);

	vec3 front_box = vec3(0.0,0.0,-1.0);
	mat3 modelMtx = inverse(transpose(mat3(M)));
	front_box = modelMtx*front_box;

	if(dot(world_n, front_box) < angle)
		discard;
		
	mat3 viewMtx = inverse(transpose(mat3(M)));
	vec3 view_t = viewMtx * world_t;
	vec3 view_b = viewMtx * world_b;
	vec3 view_n = viewMtx * world_n;
	
	mat3 tbn = mat3(-view_t,view_b,view_n);
	view_n = normalize(tbn * (2.0 * texture(normal_txt, model_uv).xyz - 1.0));

	attr_albedo = vec4(txt.rgb, 1.0);
	attr_normal = vec4(view_n, 1.0);	
}