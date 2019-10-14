#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_norm;
layout (location = 2) in vec2 attr_uvs;
layout (location = 3) in vec3 attr_tan;
layout (location = 4) in vec3 attr_bit;
layout (location = 5) in vec4 attr_wbones;
layout (location = 6) in ivec4 attr_bones;

uniform mat4 M;
uniform mat4 M_prev;
uniform mat4 V;
uniform mat4 V_prev;
uniform mat4 P;
uniform bool mb_camera_motion = false;

const int MAX_BONES=50;
uniform mat4 bones[MAX_BONES];
uniform int num_bones;

out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vPosition;
out float vMotion;
out vec2 vUv;

void main()
{
	mat4 B = bones[attr_bones[0]] * attr_wbones[0];
	B     += bones[attr_bones[1]] * attr_wbones[1];
	B     += bones[attr_bones[2]] * attr_wbones[2];
	B     += bones[attr_bones[3]] * attr_wbones[3];
	mat4 MV = V*M*B;
	mat4 MVP_prev;
	if(mb_camera_motion)
		MVP_prev=P*V_prev*M_prev;
	else
		MVP_prev=P*V*M_prev;
	mat3 normalMtx = inverse(transpose(mat3(MV)));

	vNormal = normalize(normalMtx * attr_norm);
	vTangent = normalize(normalMtx * attr_tan);
	vBitangent = normalize(normalMtx * attr_bit);
	vPosition = (MV*vec4(attr_pos, 1.0)).xyz;
	vUv = attr_uvs;

	gl_Position = P*MV* vec4(attr_pos, 1.0);
	vec2 pos = gl_Position.xy;
	vec4 Prev_Pos = MVP_prev*vec4(attr_pos, 1.0);
	vec2 prev_pos = Prev_Pos.xy;
	vMotion = length(pos-prev_pos);
}