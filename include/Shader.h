#pragma once

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
private:
	void compileErrors(const GLuint &shader, const char* type);

	void deleteShaderProgram();
public:
	GLuint ID;

	Shader(const char* fragment_shader, const char* vertex_shader);

	void activate();

	void setUniform(const std::string& name, bool value) const;
	void setUniform(const std::string& name, int value) const;
	void setUniform(const std::string& name, float value) const;
	void setUniform(const std::string& name, const glm::vec3& value) const;
	void setUniform(const std::string& name, const glm::mat4& value) const;

	~Shader();
};