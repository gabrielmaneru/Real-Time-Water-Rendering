#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D albedo_txt;
layout (binding = 1) uniform sampler2D normal_txt;
layout (binding = 2) uniform sampler2D depth_txt;
layout (binding = 3) uniform sampler2D position_txt;

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
	//float depth = texture(depth_txt, txt_uvs).x;
	
	//vec4 point = vec4(txt_uvs.x, txt_uvs.y, depth,1.0);
	//point = inverse(P) * point;
	//point /= point.w;
	//point = inverse(V*M) * point;

	vec3 view_pos = texture(position_txt, txt_uvs).xyz;
	vec4 point = vec4(view_pos, 1.0);
	point = inverse(V*M) * point;

	if(abs(point.x) > 0.5 || abs(point.y) > 0.5 || abs(point.z) > 0.5)
		discard;

	if(mode==2)
	{
		attr_albedo = vec4(1.0);
		attr_normal = vec4(1.0);
		return;
	}

	vec2 model_uv = point.xy+vec2(0.5);
	vec4 txt = texture(albedo_txt, model_uv);
	if(txt.a < 0.5)
		discard;
	vec3 albedo = txt.rgb;
	
	vec3 dx = normalize(dFdx(view_pos));
	vec3 dy = normalize(dFdy(view_pos));
	vec3 norm = normalize(cross(dx,dy));
	if(dot(norm,normalize(vNormal)) < angle)
		discard;

	mat3 tbn = mat3(dx,dy,norm);
	norm = normalize(tbn * (2.0 * texture(normal_txt, model_uv).xyz - 1.0));

	attr_albedo = vec4(albedo, 1.0);
	attr_normal = vec4(norm, 1.0);	
}