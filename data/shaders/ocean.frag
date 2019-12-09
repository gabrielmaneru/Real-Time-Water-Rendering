#version 440

in vec3 mPosition;
in vec3 vPosition;
in vec3 mNormal;
in vec3 vNormal;
in vec2 vUv;

uniform float near;
uniform float far;
uniform mat3 Vnorm;
uniform vec3 l_dir;
uniform mat4 V;
uniform mat4 P;
layout (binding = 0) uniform samplerCube skybox_txt;
layout (binding = 1) uniform sampler2D position_txt;
layout (binding = 2) uniform sampler2D diffuse_txt;
layout (binding = 3) uniform sampler2D caustic_txt;

uniform bool line_render;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out float attr_lindepth;

vec4 get_vpos(vec2 txt_uv)
{
	return texture(position_txt, txt_uv);
}
vec4 get_prev_diff(vec2 dUv)
{
	vec2 size = vec2(textureSize(diffuse_txt,0));
	vec2 txt_uvs = vec2(gl_FragCoord.x/size.x, gl_FragCoord.y/size.y)+dUv;
	if(txt_uvs.x < 0)
		txt_uvs.x = - txt_uvs.x;
	if(txt_uvs.y < 0)
		txt_uvs.y = - txt_uvs.y;
	if(txt_uvs.x > 1)
		txt_uvs.x = 2 - txt_uvs.x;
	if(txt_uvs.y > 1)
		txt_uvs.y = 2 - txt_uvs.y;
	return texture(diffuse_txt, txt_uvs);
}
const float step = 0.05;
const float max_steps = 80;
const int num_binary = 10;
vec3 raymarch(vec3 dir, vec3 hit_pos)
{
	dir *= step;
	vec4 proj;
	float z_hit;

	for(int i = 0; i < max_steps; i++)
	{
		hit_pos += dir;
		proj = P * vec4(hit_pos, 1.0);
		proj.xy /= proj.w;
		proj.xy = proj.xy * 0.5 + 0.5;
		if(proj.x < 0 || proj.x >= 1 || proj.y < 0 || proj.y >= 1)
			return vec3(0.0);
		z_hit = get_vpos(proj.xy).z;
		float delta_z = hit_pos.z - z_hit;

		if((dir.z - delta_z) < 1.2)
		{
			if(delta_z <= 0.0)
			{
				for(int j = 0; j < num_binary; j++)
				{
					dir *= 0.5;
					if(delta_z > 0.0)
						hit_pos += dir;
					else
						hit_pos -= dir;

					proj = P * vec4(hit_pos, 1.0);
					proj.xy /= proj.w;
					proj.xy = proj.xy * 0.5 + 0.5;
					z_hit = get_vpos(proj.xy).z;
					delta_z = hit_pos.z - z_hit;
				}
				return vec3(proj.xy, 1.0);
			}
		}
	}
	return vec3(0.0);
}
vec3 get_reflected_color(vec3 vPosition, vec3 vReflect_View, out bool hitted)
{
	vec3 result = raymarch(vReflect_View, vPosition);
	if(result.z == 1.0)
	{
		hitted = true;
		return texture(diffuse_txt, result.xy).rgb;
	}
	else
	{
		hitted = false;
		return vec3(0,0,0);
	}
}
vec3 get_sky_color(vec3 vReflect_View)
{
	vec3 wReflect_View = normalize(inverse(Vnorm)*vReflect_View);
	if(wReflect_View.y < 0) 
		return vec3(0.9);
	else
		return texture(skybox_txt, wReflect_View).rgb;
}
float get_caustic(vec2 world_uv)
{
	if(world_uv.x < 0.0 || world_uv.x >= 1.0 || world_uv.y < 0.0 || world_uv.y >=1.0)
		return 1.0;
	return texture(caustic_txt, world_uv).r;
}

void main()
{
	if(line_render)
	{
		attr_albedo = vec4(0.0, 0.0, 0.0, 1.0);
		attr_normal = vec4(normalize(vNormal), 1.0);
		return;
	}
	
	vec2 size = vec2(textureSize(position_txt,0));
	vec2 txt_uv = vec2(gl_FragCoord.x/size.x, gl_FragCoord.y/size.y);
	vec4 vpos = get_vpos(txt_uv);
	vec2 world_uv = (inverse(V)*vec4(vpos.xyz, 1.0)).xz/256 + 0.5;
	const float sea_view = 50.0f;
	float water_len = length(vPosition-vpos.xyz);
	if(vpos.w < 0.5)
	{
		water_len = sea_view;
		world_uv = vec2(0);
	}

	float depth_factor = clamp(water_len/sea_view,0.0,1.0);
	vec3 blue_water_color = vec3(0.0,0.2,1.0);
	vec3 green_water_color = vec3(0.0,1.0,0.2);


	vec3 vView = normalize(vPosition);
	vec3 vReflect_View = normalize(reflect(vView, vNormal));
	bool hitted;
	vec3 reflected_color = get_reflected_color(vPosition, vReflect_View, hitted);
	if(!hitted)
		reflected_color = get_sky_color(vReflect_View);


	vec3 vRefract_View = normalize(refract(vView, vNormal, 0.75));
	float refract_factor = pow(depth_factor,0.8);
	vec2 delta_Refract = (vRefract_View.xy-vView.xy)*refract_factor*0;
	vec3 prev_color = get_prev_diff(delta_Refract).xyz;
	float caustic = pow(get_caustic(world_uv),16);
	prev_color *= clamp(pow(caustic, 0.2), 0.8, 1.0);


	float drag_factor = pow(depth_factor,0.5);
	vec3 drag_color = mix(green_water_color,blue_water_color,drag_factor);
	float color_factor = pow(depth_factor,0.8);
	vec3 diffuse = mix(prev_color,drag_color,color_factor);
	//diffuse += 0.5*reflected_color;
	float l_factor = max(pow(dot(vNormal,normalize(l_dir)),2),0);
	vec3 r_light = normalize(reflect(-normalize(l_dir),vNormal));
	float s_factor = pow(max(dot(r_light,-vView),0),4);
	diffuse +=reflected_color * (mix(0.0,0.75,l_factor)+s_factor);

	attr_albedo = vec4(diffuse, 1.0);
	attr_metallic = vec4(vec3(0.0), 1.0);
	attr_normal = vec4(normalize(vNormal), 1.0);
	attr_position = vec4(vPosition, 2.0);
    float d = (near * far) / (far - near + vPosition.z);
	attr_lindepth = 1.0f-pow(1.0f-d,10);
}