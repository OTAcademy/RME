#ifndef RME_RENDERING_CORE_SHADER_PROGRAM_H_
#define RME_RENDERING_CORE_SHADER_PROGRAM_H_

#include "app/main.h"
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class ShaderProgram {
public:
	ShaderProgram();
	~ShaderProgram();

	bool Load(const std::string& vertexSource, const std::string& fragmentSource);
	void Use() const;
	void Unuse() const;

	// Uniform setters
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVec2(const std::string& name, const glm::vec2& value) const;
	void SetVec3(const std::string& name, const glm::vec3& value) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetMat4(const std::string& name, const glm::mat4& value) const;

	bool IsValid() const {
		return program_id != 0;
	}

	GLuint GetID() const {
		return program_id;
	}

private:
	GLuint program_id;
	mutable std::unordered_map<std::string, GLint> uniform_cache;

	GLint GetUniformLocation(const std::string& name) const;
	GLuint CompileShader(GLenum type, const std::string& source);
};

#endif
