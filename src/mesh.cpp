#include "mesh.h"

Mesh::Mesh(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, std::vector<GLushort>& pitchYaw, std::vector<glm::vec3> offsets) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &instanceVBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &pitchYawVBO);

	instanceCount = offsets.size();

	glBindVertexArray(VAO);
	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	// vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Pitch and Yaw offset buffer
	glBindBuffer(GL_ARRAY_BUFFER, pitchYawVBO);
	glBufferData(GL_ARRAY_BUFFER, pitchYaw.size() * sizeof(GLushort), pitchYaw.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2 * sizeof(GLushort), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 0);

	//Instance offset buffer
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec3), offsets.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::Draw(const std::vector<GLuint>& indices) {
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
	glBindVertexArray(0);
}

void Mesh::Delete() {
	if (VAO != 0)
		glDeleteVertexArrays(1, &VAO);
	if (VBO != 0)
		glDeleteBuffers(1, &VBO);
	if (EBO != 0)
		glDeleteBuffers(1, &EBO);
	if (instanceVBO != 0)
		glDeleteBuffers(1, &instanceVBO);
	if (pitchYawVBO != 0) {
		glDeleteBuffers(1, &pitchYawVBO);
	}
	instanceCount = 0;
}

Mesh::~Mesh() {
	Delete();
}

Mesh::Mesh(Mesh&& other) noexcept {
	VAO = other.VAO;
	VBO = other.VBO;
	EBO = other.EBO;
	instanceVBO = other.instanceVBO;
	instanceCount = other.instanceCount;

	// Reset source
	other.VAO = other.VBO = other.EBO = other.instanceVBO = other.instanceCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	if (this != &other) {
		// Free existing resources if necessary
		Delete();

		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		instanceVBO = other.instanceVBO;
		instanceCount = other.instanceCount;

		other.VAO = other.VBO = other.EBO = other.instanceVBO = other.instanceCount = 0;
	}
	return *this;
}