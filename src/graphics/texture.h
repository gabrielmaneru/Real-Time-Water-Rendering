#pragma once
#include <string>
struct Texture
{
	unsigned int m_id{ 0 };
	int m_width;
	int m_height;
	enum class e_texture_type {
		DIFFUSE,
		METALNESS,
		REFLECTION,
		NORMAL,
		ROUGHNESS,
		AO
	} m_type;
	std::string m_path;
};