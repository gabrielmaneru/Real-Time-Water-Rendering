#version 440

in vec3 mPosition;
in vec3 vPosition;
in vec3 mNormal;
in vec3 vNormal;
in vec2 vUv;

uniform float near;
uniform float far;
uniform mat4 V;
layout (binding = 0) uniform samplerCube skybox_txt;

uniform bool line_render;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out float attr_lindepth;

void main()
{
	if(line_render)
	{
		attr_albedo = vec4(0.0, 0.0, 0.0, 1.0);
		attr_normal = vec4(normalize(vNormal), 1.0);
		return;
	}

	vec3 cam_w = vec3(inverse(V) * vec4(0,0,0,1));
	vec3 view = normalize(mPosition-cam_w);
	vec3 to_sky = normalize(reflect(view, mNormal));
	float incline = -view.y;
	vec3 sky_color = 20*incline*texture(skybox_txt, to_sky).rgb;
	
	float border = 100*pow(dot(view,to_sky),16);
	vec3 water_color = vec3(0,1,2);

	attr_albedo = vec4(border*sky_color + (1-border)* water_color, 1.0);

	attr_metallic = vec4(vec3(0.0), 1.0);
	attr_normal = vec4(normalize(vNormal), 1.0);
	attr_position = vec4(vPosition, 2.0);
    float d = (near * far) / (far - near + vPosition.z);
	attr_lindepth = 1.0f-pow(1.0f-d,10);
}