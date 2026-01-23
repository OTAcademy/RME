#ifndef RME_RENDERING_CORE_PERSISTENT_BUFFER_H_
#define RME_RENDERING_CORE_PERSISTENT_BUFFER_H_

#include "main.h"
#include <vector>

struct BufferRange {
	size_t start;
	size_t size;
	GLsync fence;
};

class PersistentBuffer {
public:
	PersistentBuffer(size_t size, GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	~PersistentBuffer();

	void Init();
	void Release();

	// Allocate space in the ring buffer.
	// Returns the mapped pointer to write to, and sets out_offset for bind commands.
	void* Allocate(size_t size, size_t& out_offset);

	// Mark the recently allocated range as "in use by GPU" after issuing draw command.
	void Lock(size_t size);

	GLuint GetID() const {
		return bufferID;
	}

private:
	void WaitForFence(size_t start, size_t size);

	GLuint bufferID = 0;
	size_t totalSize = 0;
	size_t currentOffset = 0;
	uint8_t* mappedData = nullptr;
	GLbitfield flags;

	std::vector<BufferRange> fences;
};

#endif
