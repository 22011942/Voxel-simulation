#pragma once

#include <iostream>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
	GLubyte x, y, z;
	GLushort pitch, yaw;
	glm::u8vec3 localPos;
};


struct Vertex_Data {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
};

class Mesh {
public:
	Mesh(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, std::vector<GLushort>& pitchYaw, std::vector<glm::vec3> offsets);

	void validate() const;

	Mesh(const Vertex_Data& data);

	Mesh() = default;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	// Delete copy constructor and copy assignment
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// Allow move
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void instancedDraw(const std::vector<GLuint>& indices);

	void Draw();

	~Mesh();

private:
	GLuint VBO{}, VAO{}, EBO{}, instanceVBO{}, pitchYawVBO{}, instanceCount{};

	void Delete();
};