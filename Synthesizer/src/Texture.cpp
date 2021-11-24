#include "Texture.h"

#include <stb_image.h>

Texture::Texture()
{
}

Texture::Texture(const Texture& texture)
{
	id = texture.id;
	width = texture.width;
	height = texture.height;
}

Texture::Texture(Texture&& texture) noexcept
{
	id = texture.id;
	width = texture.width;
	height = texture.height;

	texture.id = 0;
}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

void Texture::writePixels(const int width, const int height, const unsigned char* pixels)
{
	if (id == 0)
		create();

	this->width = width;
	this->height = height;
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	unbind();
}

void Texture::loadFromImage(const char* source)
{
	if (id == 0)
		create();

	int channels;
	GLenum format;
	unsigned char* pixels = stbi_load(source, &width, &height, &channels, 0);
	if (pixels == nullptr)
	{
		printf("Failed to load image: %s!", source);
		return;
	}
	switch (channels)
	{
	case 1:
		format = GL_RED;
		break;
	case 2:
		format = GL_RG;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		format = GL_RED;
		break;
	}

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
	unbind();
	stbi_image_free(pixels);
}

void Texture::updateRegion(const int x, const int y, const int w, const int h, const unsigned char* pixels)
{
	bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	unbind();
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

const int Texture::getWidth() const
{
	return width;
}

const int Texture::getHeight() const
{
	return height;
}

const int Texture::getID() const
{
	return id;
}

void Texture::create()
{
	glGenTextures(1, &id);
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unbind();
}
