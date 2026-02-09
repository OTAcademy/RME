#include "rendering/core/shared_geometry.h"
#include <spdlog/spdlog.h>
#ifdef _WIN32
	#include <windows.h>
#elif defined(__linux__)
	#include <GL/glx.h>
#elif defined(__APPLE__)
	#include <OpenGL/OpenGL.h>
#endif

void* GetCurrentGLContext() {
#ifdef _WIN32
	return (void*)wglGetCurrentContext();
#elif defined(__linux__)
	return (void*)glXGetCurrentContext();
#elif defined(__APPLE__)
	return (void*)CGLGetCurrentContext();
#else
	return nullptr;
#endif
}

bool SharedGeometry::initialize() {
	void* ctx = GetCurrentGLContext();
	if (!ctx) {
		return false;
	}

	std::lock_guard<std::mutex> lock(mutex_);

	if (contexts_.find(ctx) != contexts_.end()) {
		return true;
	}

	auto& geom = contexts_[ctx];
	geom.vbo = std::make_unique<GLBuffer>();
	geom.ebo = std::make_unique<GLBuffer>();

	// Unit quad geometry (0..1)
	// Pos (vec2), Tex (vec2)
	// Composed of 4 vertices
	float quad_vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f, // pos.xy, tex.xy
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	// Upload static geometry (Immutable storage)
	glNamedBufferStorage(geom.vbo->GetID(), sizeof(quad_vertices), quad_vertices, 0);
	glNamedBufferStorage(geom.ebo->GetID(), sizeof(indices), indices, 0);

	spdlog::info("SharedGeometry: initialized for context {} (VBO: {}, EBO: {})", ctx, geom.vbo->GetID(), geom.ebo->GetID());
	return true;
}

GLuint SharedGeometry::getQuadVBO() {
	initialize();
	void* ctx = GetCurrentGLContext();
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = contexts_.find(ctx);
	if (it != contexts_.end()) {
		return it->second.vbo->GetID();
	}
	return 0;
}

GLuint SharedGeometry::getQuadEBO() {
	initialize();
	void* ctx = GetCurrentGLContext();
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = contexts_.find(ctx);
	if (it != contexts_.end()) {
		return it->second.ebo->GetID();
	}
	return 0;
}
