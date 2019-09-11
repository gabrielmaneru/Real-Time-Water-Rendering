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
	unsigned char* data = stbi_load((std::string(".") + str).c_str(), &m_width, &m_height, &nrChannels, 0);
	if (data)
	{
		GLenum format, internalFormat;
		if (nrChannels == 1) {
			format = internalFormat = GL_RED;
		}

		else if (nrChannels == 3) {
			if (gamma_correction)
				internalFormat = GL_SRGB;
			else
				internalFormat = GL_RGB;

			format = GL_RGB;
		}

		else if (nrChannels == 4) {
			if (gamma_correction)
				internalFormat = GL_SRGB8_ALPHA8;
			else
				internalFormat = GL_RGBA;

			format = GL_RGBA;
		}

		// Load data into the current bound texture
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else
		throw std::string("Image not loaded: ") + str;

	// Free the image data
	stbi_image_free(data);
}
