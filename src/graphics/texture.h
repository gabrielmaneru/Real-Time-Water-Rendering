#pragma once
#include <string>
struct Texture
{
	unsigned int m_id{ 0 };
	int m_width;
	int m_height;
	void loadFromFile(const char * str, bool gamma_correction);
	enum e_texture_type
	{
		DIFFUSE = 0x1,
		SPECULAR = 0x2,
		NORMAL = 0x5,
	} m_type;
	std::string m_path;
};