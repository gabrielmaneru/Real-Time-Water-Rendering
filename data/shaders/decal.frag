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
	vec4 model_point = inverse(V) * view_point;
	model_point = inverse(M) * model_point;

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
	
	vec3 dx = normalize(dFdx(view_point.xyz));
	vec3 dy = normalize(dFdy(view_point.xyz));
	vec3 norm = normalize(cross(dx,dy));

	vec3 front_box = vec3(0.0,0.0,-1.0);
	mat3 normalMtx = inverse(transpose(mat3(V*M)));
	front_box = normalMtx*front_box;

	if(dot(norm,normalize(front_box)) < angle)
		discard;

	mat3 tbn = mat3(-dx,dy,norm);
	norm = normalize(tbn * (2.0 * texture(normal_txt, model_uv).xyz - 1.0));

	attr_albedo = vec4(txt.rgb, 1.0);
	attr_normal = vec4(norm, 1.0);	
}