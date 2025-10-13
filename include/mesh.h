#pragma once

#include <iostream>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
	Mesh(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, std::vector<GLushort>& pitchYaw, std::vector<glm::vec3> offsets);

	Mesh() = default;

	// Delete copy constructor and copy assignment
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// Allow move
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void Draw(const std::vector<GLuint>& indices);

	~Mesh();

private:
	GLuint VBO, VAO, EBO, instanceVBO, pitchYawVBO, instanceCount;

	void Delete();
};