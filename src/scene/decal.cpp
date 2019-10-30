#include "decal.h"

decal::decal(std::string diffuse, std::string normal, transform3d tr)
	:scene_object("cube", tr,nullptr,nullptr)
{
	m_diffuse.m_path = Texture::filter_name(diffuse);
	m_diffuse.loadFromFile(m_diffuse.m_path.c_str(), false);

	m_normal.m_path = Texture::filter_name(normal);
	m_normal.loadFromFile(m_normal.m_path.c_str(), false);
}
