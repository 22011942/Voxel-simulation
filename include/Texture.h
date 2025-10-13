#pragma once

#include <iostream>
#include "stb_image.h"
#include <glad/glad.h>

class Texture {
private:

public:
	GLuint ID;

	GLenum texType;

	Texture(const char* filename, GLenum texType, GLenum format, GLenum pixelType);

	void bindTexture();

	void deleteTexture();

	~Texture();
};