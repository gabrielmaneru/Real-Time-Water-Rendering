#version 440
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vPosition;
in float vMotion;
in vec2 vUv;

layout (binding = 0) uniform sampler2D position_txt;
layout (binding = 1) uniform sampler2D albedo_txt;
layout (binding = 2) uniform sampler2D metallic_txt;
layout (binding = 3) uniform sampler2D normal_txt;
uniform vec3 l_pos;
uniform vec3 la;
uniform vec3 ld;
uniform vec3 att_factor;
uniform float rad;
uniform int window_width;
uniform int window_height;
subroutine void Render_Type();
subroutine uniform Render_Type render_pass;

out vec3 out_color;

subroutine (Render_Type)
void render_ambient()
{
	vec2 new_uvs = vec2(gl_FragCoord.x/window_width, gl_FragCoord.y/window_height);
	
	vec4 albedo_value = texture(albedo_txt, new_uvs);
	vec4 position_value = texture(position_txt, new_uvs);
	vec4 normal_value = texture(normal_txt, new_uvs);
	if(normal_value.xyz == vec3(0,0,0))
		discard;
	if(normal_value.xyz == vec3(1,1,1))
		out_color=vec3(1.0);
	else
	{
		vec3 kd = albedo_value.rgb;
		float ka = position_value.a-1.0;
		out_color = (ka*la)*kd;
	}
}

//////////////////////////////////////////////////////////////////////////
//PBR-by-Jon-Sanchez-based-on-Naty-Hoffman's-Paper////////////////////////
//////////////////////////////////////////////////////////////////////////
	float PI = 3.141592653589;
	vec3 FresnelSchlickUnreal(float cosTheta, vec3 F0)
	{
		return mix(F0, vec3(1.0), pow(2.0, -5.55473 * cosTheta - 6.98316 * cosTheta));
	}

	vec3 FresnelSchlick(float cosTheta, vec3 F0)
	{
		return mix(F0, vec3(1.0), pow(1.0 - cosTheta, 5.0));
	}

	float DistributionBlinn(float NdotH, float roughness)
	{
		float m = roughness * roughness;
		float m2 = m * m;
    
		float n = 2.0 / m2 - 2.0;
		return (n + 2.0) / (2.0 * PI) * pow(NdotH, n);
	}

	float DistributionBeckmann(float NdotH, float roughness)
	{
		float m = roughness * roughness;
		float m2 = m * m;
		float NdotH2 = NdotH * NdotH;
    
		return exp((NdotH2 - 1.0) / (m2 * NdotH2)) / (PI * m2 * NdotH2 * NdotH2);
	}

	float DistributionGGX(float NdotH, float roughness)
	{
		float m = roughness*roughness;
		float m2 = m*m;
	
		float d = (NdotH * NdotH) * (m2 - 1.0) + 1.0;
		return m2 / (PI * d * d);
	}

	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r*r) / 8.0;

		float num   = NdotV;
		float denom = NdotV * (1.0 - k) + k;
	
		return num / denom;
	}
	float GeometrySmith(float NdotV, float NdotL, float roughness)
	{
		float ggx2  = GeometrySchlickGGX(NdotV, roughness);
		float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
		return ggx1 * ggx2;
	}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vec3 pointlight(vec3 albedo, vec3 metallic, float roughness, vec3 pos, vec3 norm) 
{
	vec3 Norm = normalize(norm);
	vec3 Light = (l_pos - pos);
	vec3 View = normalize(-pos);

	float d = length(Light);
	if(d>rad) discard;
	float att = min(1/(att_factor.x + att_factor.y*d + att_factor.z*d*d) , 1.0);
	Light = normalize(Light);
	
	vec3 Half = normalize(View + Light);
    
	float NdotV = max(dot(Norm, View), 0.001);
	float NdotL = max(dot(Norm, Light), 0.001);
	float NdotH = max(dot(Norm, Half), 0.001);
	float VdotH = max(dot(View, Half), 0.001);
    
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
	float NDF = DistributionGGX(NdotH, roughness);       
	float G = GeometrySmith(NdotV, NdotL, roughness);  
	vec3 F = FresnelSchlick(NdotH, F0);
        
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
    
	vec3 diffuse = albedo * kD / PI;
	float denominator = 4.0 * NdotV * NdotL;
	vec3 specular = (NDF * G * F) / max(denominator, 0.001); 
      
	return (specular + diffuse) * (5.0f*ld) * NdotL * att;
}

subroutine (Render_Type)
void render_diffuse_specular()
{
	vec2 new_uvs = vec2(gl_FragCoord.x/window_width, gl_FragCoord.y/window_height);
	
	vec4 albedo_value = texture(albedo_txt, new_uvs);
	vec4 position_value = texture(position_txt, new_uvs);
	vec4 metallic_value = texture(metallic_txt, new_uvs);
	vec4 normal_value = texture(normal_txt, new_uvs);

	if(normal_value.xyz == vec3(0,0,0))
		discard;
		
	vec3 frag_pos = position_value.rgb;
	vec3 albedo = albedo_value.rgb;
	float roughness = albedo_value.a-1.0;
	vec3 metallic = metallic_value.rgb;
	vec3 normal = normalize(normal_value.rgb);

	out_color = pointlight(albedo, metallic, roughness, frag_pos, normal);
}

void main()
{
	render_pass();
}