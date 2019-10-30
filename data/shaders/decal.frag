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

	vec4 point = vec4(txt_uvs.x, txt_uvs.y, depth,1.0);
	point = inverse(P) * point;
	point /= point.w;
	point = inverse(V*M) * point;

	//if(abs(point.x) > 0.5 || abs(point.y) > 0.5 || abs(point.z) > 0.5)
	//	discard;

	if(mode==2)
	{
		attr_albedo = vec4(1.0);
		attr_normal = vec4(1.0);
		return;
	}

	vec2 model_uv = point.xy;
	vec4 txt = texture(albedo_txt, model_uv);
	if(txt.a < 0.5)
		discard;
	vec3 albedo = txt.rgb;
	
	const vec3 T = normalize(vTangent);
	const vec3 B = normalize(vBitangent);
	const vec3 N = normalize(vNormal);
	mat3 TBN = mat3(T,B,N);
	vec3 normal = normalize(TBN * (2.0 * texture(normal_txt, model_uv).xyz - 1.0));

	if(dot(N,N) < angle)
		discard;

	attr_albedo = vec4(albedo, 1.0);
	attr_normal = vec4(normal, 1.0);	
}