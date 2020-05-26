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
layout (binding = 4) uniform sampler2D foam_txt;

uniform bool WireframeMode;
uniform float ShoreDistance;
uniform float ShoreColorPower;
uniform vec3 ShoreWaterColor;
uniform vec3 DeepWaterColor;
uniform float ShoreBlendPower;
uniform bool DoReflection;
uniform float ReflectionStep;
uniform int ReflectionStepMax;
uniform int ReflectionRefinementCount;
uniform float ReflectionMaxPen;
uniform bool DoRefraction;
uniform float RefractionAngle;
uniform bool DoCaustic;
uniform float CausticPower;
uniform vec2 CausticInterval;
uniform bool DoLighting;
uniform vec2 LightInterval;
uniform float LightSpecular;
uniform bool DoFoam;
uniform vec2 FoamInterval;

layout (location = 0) out vec4 attr_position;
layout (location = 1) out vec4 attr_albedo;
layout (location = 2) out vec4 attr_metallic;
layout (location = 3) out vec4 attr_normal;
layout (location = 4) out float attr_lindepth;

vec4 get_vPrevPos(vec2 ScrUv)
{
	return texture(position_txt, ScrUv);
}
vec4 get_PrevColor(vec2 uv)
{
	if(uv.x < 0)
		uv.x = - uv.x;
	if(uv.y < 0)
		uv.y = - uv.y;
	if(uv.x > 1)
		uv.x = 2 - uv.x;
	if(uv.y > 1)
		uv.y = 2 - uv.y;
	return texture(diffuse_txt, uv);
}
vec3 RaymarchPosition(vec3 vReflectView, vec3 vRayPos)
{
	vReflectView *= ReflectionStep;
	vec4 HitUv;
	float vHitZ;

	for(int i = 0; i < ReflectionStepMax; i++)
	{
		vRayPos += vReflectView;
		HitUv = P * vec4(vRayPos, 1.0);
		HitUv.xy /= HitUv.w;
		HitUv.xy = HitUv.xy * 0.5 + 0.5;
		if(HitUv.x < 0 || HitUv.x >= 1 || HitUv.y < 0 || HitUv.y >= 1)
			return vec3(0.0);
		vHitZ = get_vPrevPos(HitUv.xy).z;
		float delta_z = vRayPos.z - vHitZ;
		float pen = vReflectView.z - delta_z;
		if(pen <= ReflectionMaxPen)
		{
			if(delta_z <= 0.0)
			{
				for(int j = 0; j < ReflectionRefinementCount; j++)
				{
					vReflectView *= 0.5;
					if(delta_z > 0.0)
						vRayPos += vReflectView;
					else
						vRayPos -= vReflectView;

					HitUv = P * vec4(vRayPos, 1.0);
					HitUv.xy /= HitUv.w;
					HitUv.xy = HitUv.xy * 0.5 + 0.5;
					vHitZ = get_vPrevPos(HitUv.xy).z;
					delta_z = vRayPos.z - vHitZ;
				}
				return vec3(HitUv.xy, 1-pow(clamp(pen/ReflectionMaxPen,0,1),2));
			}
		}
	}
	return vec3(0.0);
}
vec3 get_sky_color(vec3 vReflectView)
{
	vec3 wReflect_View = normalize(inverse(Vnorm)*vReflectView);
	if(wReflect_View.y < 0) 
		return vec3(0.9);
	else
		return texture(skybox_txt, wReflect_View).rgb;
}
vec3 get_ReflectedColor(vec3 vPosition, vec3 vReflectView)
{
	vec3 result = RaymarchPosition(vReflectView, vPosition);
	return mix(get_sky_color(vReflectView),texture(diffuse_txt, result.xy).rgb,result.z);
}
float get_Caustic(vec2 wUv)
{
	if(wUv.x < 0.0 || wUv.x >= 1.0 || wUv.y < 0.0 || wUv.y >=1.0)
		return 1.0;
	return texture(caustic_txt, wUv).r;
}

void main()
{
	if(WireframeMode)
	{
		attr_albedo = vec4(0.0, 0.0, 0.0, 1.0);
		attr_normal = vec4(normalize(vNormal), 1.0);
		return;
	}
	
	vec2 TxtSize = vec2(textureSize(position_txt,0));
	vec2 ScrUv = vec2(gl_FragCoord.x/TxtSize.x, gl_FragCoord.y/TxtSize.y);

	vec4 vPrevPos = get_vPrevPos(ScrUv);
	vec2 wUv = (inverse(V)*vec4(vPrevPos.xyz, 1.0)).xz/256 + 0.5;
	float ShoreLength = length(vPosition-vPrevPos.xyz);
	if(vPrevPos.w < 0.5)
	{
		ShoreLength = ShoreDistance;
		wUv = vec2(0);
	}
	float ShoreFactor = clamp(ShoreLength/ShoreDistance,0.0,1.0);

	vec3 vView = normalize(vPosition);
	vec3 vReflectView = normalize(reflect(vView, vNormal));
	vec3 ReflectedColor = DoReflection ? get_ReflectedColor(vPosition, vReflectView) : vec3(1);

	vec3 vRefractView = normalize(refract(vView, vNormal, RefractionAngle));
	vec2 dRefractUv = DoRefraction ? (vRefractView.xy-vView.xy)*ShoreFactor : vec2(0);
	vec3 RefractedColor = get_PrevColor(ScrUv + dRefractUv).xyz;

	if(DoCaustic)
	{
		float CausticValue = pow(get_Caustic(wUv),CausticPower);
		RefractedColor *= mix(1.0,mix(CausticInterval.x, CausticInterval.y,CausticValue),ShoreFactor);
	}

	float ShoreColorFactor = pow(ShoreFactor,ShoreColorPower);
	vec3 ShoreColor = mix(ShoreWaterColor,DeepWaterColor,ShoreColorFactor);
	float ShoreBlendFactor = pow(ShoreFactor,ShoreBlendPower);
	vec3 WaterColor = mix(RefractedColor,ShoreColor,ShoreBlendFactor);

	float LightFactor = max(pow(dot(vNormal,normalize(l_dir)),0.1),0);
	vec3 ReflectedLight = normalize(reflect(normalize(-l_dir),vNormal));
	float SpecularFactor = pow(max(dot(ReflectedLight,-vView),0),2);
	float light = DoLighting ? mix(LightInterval.x,LightInterval.y,LightFactor) + LightSpecular*SpecularFactor : mix(LightInterval.x,LightInterval.y,0.5);
	
	float n_rat = 1.0 / 1.3333;
	float u_rat = 1.0 / 1.3333;
	float R_factor = 1.0;

	// Fresnel Equation
	if(n_rat > 0.01)
	{
		float cosI = dot(-vView, vNormal);
		float sinI = 1.0 - cosI * cosI;
		float in_root = 1.0 - n_rat * n_rat * sinI;
		if(in_root > 0.0)
		{
			float root = sqrt(in_root);
			float e_perp = (n_rat*cosI - u_rat*root) / (n_rat*cosI + u_rat*root);
			float e_para = (u_rat*cosI - n_rat*root) / (u_rat*cosI + n_rat*root);
			R_factor = 0.5 * (e_perp*e_perp + e_para*e_para);
		}
	}

	if(DoReflection)
		WaterColor = mix(WaterColor,ReflectedColor,R_factor);
	if(DoLighting)
		WaterColor *= light;
	if(DoFoam)
	{
		float foam = clamp(smoothstep(FoamInterval.x,FoamInterval.y,mPosition.y),0,1);
		float foam_color = length(texture(foam_txt, vUv*15).rgb);
		if(foam_color> 0.8)
			WaterColor = mix(WaterColor,vec3(foam_color),foam);
	}

	attr_albedo = vec4(WaterColor, 1.0);
	attr_metallic = vec4(vec3(0.0), 1.0);
	attr_normal = vec4(normalize(vNormal), 1.0);
	attr_position = vec4(vPosition, 2.0);
    float d = (near * far) / (far - near + vPosition.z);
	attr_lindepth = 1.0f-pow(1.0f-d,10);
}