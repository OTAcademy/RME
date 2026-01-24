#include "rendering/core/shader_program.h"
#include <vector>
#include <iostream>

ShaderProgram::ShaderProgram() :
	program_id(0) {
}

ShaderProgram::~ShaderProgram() {
	if (program_id != 0) {
		glDeleteProgram(program_id);
	}
}

bool ShaderProgram::Load(const std::string& vertexSource, const std::string& fragmentSource) {
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	if (vertexShader == 0) {
		return false;
	}

	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (fragmentShader == 0) {
		glDeleteShader(vertexShader);
		return false;
	}

	program_id = glCreateProgram();
	glAttachShader(program_id, vertexShader);
	glAttachShader(program_id, fragmentShader);
	glLinkProgram(program_id);

	GLint success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program_id, 1024, NULL, infoLog);
		std::cerr << "SHADER PROGRAM LINKING ERROR: " << infoLog << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return true;
}

void ShaderProgram::Use() const {
	if (program_id != 0) {
		glUseProgram(program_id);
	}
}

void ShaderProgram::Unuse() const {
	glUseProgram(0);
}

GLint ShaderProgram::GetUniformLocation(const std::string& name) const {
	if (uniform_cache.find(name) != uniform_cache.end()) {
		return uniform_cache[name];
	}

	GLint location = glGetUniformLocation(program_id, name.c_str());
	if (location == -1) {
		// std::cout << "Warning: Uniform '" << name << "' doesn't exist!" << std::endl;
	}

	uniform_cache[name] = location;
	return location;
}

void ShaderProgram::SetBool(const std::string& name, bool value) const {
	glUniform1i(GetUniformLocation(name), (int)value);
}

void ShaderProgram::SetInt(const std::string& name, int value) const {
	glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetFloat(const std::string& name, float value) const {
	glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetVec2(const std::string& name, const glm::vec2& value) const {
	glUniform2fv(GetUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::SetVec3(const std::string& name, const glm::vec3& value) const {
	glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::SetVec4(const std::string& name, const glm::vec4& value) const {
	glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::SetMat4(const std::string& name, const glm::mat4& value) const {
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

GLuint ShaderProgram::CompileShader(GLenum type, const std::string& source) {
	GLuint shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		std::cerr << "SHADER COMPILATION ERROR (" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "):\n"
				  << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
