#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"

Texture::Texture(const char* filename, GLenum texType, GLenum format, GLenum pixelType) : texType{ texType } {
	glGenTextures(1, &ID);
	glBindTexture(texType, ID);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint width, height, nrChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	if (data) {
		glTexImage2D(texType, 0, format, width, height, 0, format, pixelType, data);
		glGenerateMipmap(texType);
	}
	else {
		std::cerr << "Failed to load texture: " << stbi_failure_reason() << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(texType, 0);
}

void Texture::bindTexture() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texType, ID);
}

void Texture::deleteTexture() {
	glDeleteTextures(1, &ID);
}

Texture::~Texture() {
	deleteTexture();
}