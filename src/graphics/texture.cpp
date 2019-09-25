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

void Texture::loadFromFile(const char * str, bool gamma_correction)
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
