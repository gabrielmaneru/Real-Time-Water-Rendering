#version 440
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_norm;
layout (location = 2) in vec2 attr_uvs;
layout (location = 3) in vec3 attr_tan;
layout (location = 4) in vec3 attr_bit;
layout (location = 5) in vec4 attr_wbones;
layout (location = 6) in ivec4 attr_bones;

out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec2 vUv;

void main()
{
    gl_Position = vec4(attr_pos, 1.0);

    vNormal = attr_norm;
    vTangent = attr_tan;
    vBitangent = attr_bit;
    vUv = attr_uvs;
}