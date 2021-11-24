#pragma once

#include <glad/glad.h>

class Texture
{
public:
	Texture();
	Texture(const Texture& texture);
	Texture(Texture&& texture) noexcept;
	~Texture();

	void writePixels(const int width, const int height, const unsigned char* pixels);
	void loadFromImage(const char* source);
	void updateRegion(const int x, const int y, const int w, const int h, const unsigned char* pixels);
	void bind() const;
	void unbind() const;

	const int getWidth() const;
	const int getHeight() const;
	const int getID() const;

private:
	void create();

private:
	GLuint id = 0;
	int width, height;
};