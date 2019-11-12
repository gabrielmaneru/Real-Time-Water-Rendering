/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	texture.cpp
Purpose: Basic texture handler
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "texture.h"
#include <gl/gl3w.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

void Texture::loadFromFile(const char * str, bool clamp)
{
	// Generate and bind texture
	glGenTextures(1, &m_id);

	// Load textures
	int nrChannels;
	unsigned char* data = stbi_load(str, &m_width, &m_height, &nrChannels, 0);
	
	if (data)
	{
		GLenum format, internalFormat;
		if (nrChannels == 1)
			format = internalFormat = GL_RED;
		else if (nrChannels == 3)
			format = internalFormat = GL_RGB;
		else if (nrChannels == 4)
			format = internalFormat = GL_RGBA;

		// Load data into the current bound texture
		glBindTexture(GL_TEXTURE_2D, m_id);

		// Set texture parameters
		auto md = clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, md);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, md);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else
		throw std::string("Image not loaded: ") + str;

	// Free the image data
	stbi_image_free(data);
}

void Texture::loadCubemapFromFile(std::vector<const char*> targets)
{
	// Generate and bind texture
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	// Load textures
	for(GLuint i = 0; i < 6; i++)
	{
		int nrChannels;
		unsigned char* data = stbi_load(targets[i], &m_width, &m_height, &nrChannels, 0);
		if(data==nullptr)
			throw std::string("Image not loaded: ") + targets[i];

		GLenum format, internalFormat;
		if (nrChannels == 1)
			format = internalFormat = GL_RED;
		else if (nrChannels == 3)
			format = internalFormat = GL_RGB;
		else if (nrChannels == 4)
			format = internalFormat = GL_RGBA;

		// Set texture parameters

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::string Texture::filter_name(const std::string& name)
{
	size_t start = name.find_last_of('/') + 1;
	if (start > name.size()) start = 0;
	if (start == 0)
	{
		start = name.find_last_of('\\') + 1;
		if (start > name.size()) start = 0;
	}
	return "./data/textures/" + name.substr(start);
}
