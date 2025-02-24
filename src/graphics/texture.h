/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	texture.h
Purpose: Basic texture handler
Author: Gabriel Ma�eru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include <string>
#include <vector>
struct Texture
{
	unsigned int m_id{ 0 };
	int m_width;
	int m_height;
	void loadFromFile(const char * str, bool clamp = false);
	void loadCubemapFromFile(std::vector<const char *> targets);
	static std::string filter_name(const std::string&);
	std::string m_path;
};