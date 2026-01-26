#ifndef RME_RENDERING_CORE_GL_RESOURCES_H_
#define RME_RENDERING_CORE_GL_RESOURCES_H_

#include <glad/glad.h>
#include <utility> // for std::exchange
#include "rendering/core/gl_texture.h"

// RAII wrapper for OpenGL Buffers (VBO, EBO, UBO, SSBO)
class GLBuffer {
public:
	GLBuffer() {
		glCreateBuffers(1, &id);
	}

	~GLBuffer() {
		if (id) {
			glDeleteBuffers(1, &id);
		}
	}

	// Disable copy
	GLBuffer(const GLBuffer&) = delete;
	GLBuffer& operator=(const GLBuffer&) = delete;

	// Enable move
	GLBuffer(GLBuffer&& other) noexcept :
		id(std::exchange(other.id, 0)) { }

	GLBuffer& operator=(GLBuffer&& other) noexcept {
		if (id) {
			glDeleteBuffers(1, &id);
		}
		id = std::exchange(other.id, 0);
		return *this;
	}

	// Explicit conversion
	explicit operator GLuint() const {
		return id;
	}
	GLuint GetID() const {
		return id;
	}

private:
	GLuint id = 0;
};

// RAII wrapper for OpenGL Vertex Arrays (VAO)
class GLVertexArray {
public:
	GLVertexArray() {
		glCreateVertexArrays(1, &id);
	}

	~GLVertexArray() {
		if (id) {
			glDeleteVertexArrays(1, &id);
		}
	}

	// Disable copy
	GLVertexArray(const GLVertexArray&) = delete;
	GLVertexArray& operator=(const GLVertexArray&) = delete;

	// Enable move
	GLVertexArray(GLVertexArray&& other) noexcept :
		id(std::exchange(other.id, 0)) { }

	GLVertexArray& operator=(GLVertexArray&& other) noexcept {
		if (id) {
			glDeleteVertexArrays(1, &id);
		}
		id = std::exchange(other.id, 0);
		return *this;
	}

	// Explicit conversion
	explicit operator GLuint() const {
		return id;
	}
	GLuint GetID() const {
		return id;
	}

private:
	GLuint id = 0;
};

// RAII wrapper for OpenGL Textures
class GLTextureResource {
public:
	explicit GLTextureResource(GLenum target = GL_TEXTURE_2D) :
		target(target) {
		glCreateTextures(target, 1, &id);
	}

	~GLTextureResource() {
		if (id) {
			glDeleteTextures(1, &id);
		}
	}

	// Disable copy
	GLTextureResource(const GLTextureResource&) = delete;
	GLTextureResource& operator=(const GLTextureResource&) = delete;

	// Enable move
	GLTextureResource(GLTextureResource&& other) noexcept :
		id(std::exchange(other.id, 0)), target(other.target) { }

	GLTextureResource& operator=(GLTextureResource&& other) noexcept {
		if (id) {
			glDeleteTextures(1, &id);
		}
		id = std::exchange(other.id, 0);
		target = other.target;
		return *this;
	}

	// Explicit conversion
	explicit operator GLuint() const {
		return id;
	}
	GLuint GetID() const {
		return id;
	}

private:
	GLuint id = 0;
	GLenum target;
};

#endif
