#ifndef RME_RENDERING_CORE_SHARED_GEOMETRY_H_
#define RME_RENDERING_CORE_SHARED_GEOMETRY_H_

#include "rendering/core/gl_resources.h"
#include <memory>
#include <map>
#include <mutex>

class SharedGeometry {
public:
	static SharedGeometry& Instance() {
		static SharedGeometry instance;
		return instance;
	}

	bool initialize();
	GLuint getQuadVBO();
	GLuint getQuadEBO();

	// Returns number of indices for the quad (6)
	GLsizei getQuadIndexCount() const {
		return 6;
	}

private:
	SharedGeometry() = default;
	~SharedGeometry() = default;

	SharedGeometry(const SharedGeometry&) = delete;
	SharedGeometry& operator=(const SharedGeometry&) = delete;

	struct ContextGeometry {
		std::unique_ptr<GLBuffer> vbo;
		std::unique_ptr<GLBuffer> ebo;
	};

	std::map<void*, ContextGeometry> contexts_;
	std::recursive_mutex mutex_;
};

#endif
