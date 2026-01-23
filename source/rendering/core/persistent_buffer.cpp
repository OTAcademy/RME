#include "rendering/core/persistent_buffer.h"
#include <iostream>

PersistentBuffer::PersistentBuffer(size_t size, GLbitfield flags) : totalSize(size), flags(flags) {
}

PersistentBuffer::~PersistentBuffer() {
	Release();
}

void PersistentBuffer::Init() {
	glCreateBuffers(1, &bufferID);
	glNamedBufferStorage(bufferID, totalSize, nullptr, flags);
	mappedData = (uint8_t*)glMapNamedBufferRange(bufferID, 0, totalSize, flags);
	currentOffset = 0;
}

void PersistentBuffer::Release() {
	if (bufferID) {
		glUnmapNamedBuffer(bufferID);
		glDeleteBuffers(1, &bufferID);
		bufferID = 0;
	}

	for (auto& range : fences) {
		if (range.fence) {
			glDeleteSync(range.fence);
		}
	}
	fences.clear();
}

void* PersistentBuffer::Allocate(size_t size, size_t& out_offset) {
	if (size > totalSize) {
		std::cerr << "PersistentBuffer::Allocate: Requested size too large" << std::endl;
		return nullptr;
	}

	// Align size to 16 bytes for better performance
	size = (size + 15) & ~15;

	// Check wrap around
	if (currentOffset + size > totalSize) {
		currentOffset = 0;
	}

	// Wait for overlap (only if necessary)
	WaitForFence(currentOffset, size);

	out_offset = currentOffset;
	return mappedData + currentOffset;
}

void PersistentBuffer::Lock(size_t size) {
	// Align size to match Allocate
	size = (size + 15) & ~15;

	// Create a single fence for the entire flush
	GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	fences.push_back({ currentOffset, size, fence });
	currentOffset += size;
}

void PersistentBuffer::WaitForFence(size_t start, size_t size) {
	size_t end = start + size;

	auto it = fences.begin();
	while (it != fences.end()) {
		// First, check if the fence is already signaled (fast path - no wait)
		GLint status = 0;
		glGetSynciv(it->fence, GL_SYNC_STATUS, 1, nullptr, &status);

		if (status == GL_SIGNALED) {
			// Fence completed, remove it without waiting
			glDeleteSync(it->fence);
			it = fences.erase(it);
			continue;
		}

		// Check intersection
		size_t f_start = it->start;
		size_t f_end = it->start + it->size;
		bool overlap = (start < f_end && end > f_start);

		if (overlap) {
			// Must wait for this fence
			GLenum result = glClientWaitSync(it->fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000); // 1ms timeout
			if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
				// Good, fence completed
			} else if (result == GL_TIMEOUT_EXPIRED) {
				// Rare: GPU is still busy. Keep waiting.
				glClientWaitSync(it->fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000); // 1 sec
			}
			glDeleteSync(it->fence);
			it = fences.erase(it);
		} else {
			++it;
		}
	}
}
