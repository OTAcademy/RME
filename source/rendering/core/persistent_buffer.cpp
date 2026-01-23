#include "rendering/core/persistent_buffer.h"
#include <iostream>

PersistentBuffer::~PersistentBuffer() {
	cleanup();
}

bool PersistentBuffer::initialize(size_t element_size, size_t max_elements) {
	if (initialized_) {
		return true;
	}

	elementSize_ = element_size;
	maxElements_ = max_elements;
	sectionSize_ = element_size * max_elements;

	// Total buffer size: 3 sections
	size_t total_size = sectionSize_ * BUFFER_COUNT;

	glCreateBuffers(1, &bufferId_);
	if (bufferId_ == 0) {
		std::cerr << "PersistentBuffer: Failed to create buffer" << std::endl;
		return false;
	}

	GLbitfield storage_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glNamedBufferStorage(bufferId_, total_size, nullptr, storage_flags);

	GLbitfield map_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	mappedPtr_ = glMapNamedBufferRange(bufferId_, 0, total_size, map_flags);

	if (!mappedPtr_) {
		std::cerr << "PersistentBuffer: Persistent mapping failed" << std::endl;
		glDeleteBuffers(1, &bufferId_);
		bufferId_ = 0;
		return false;
	}

	initialized_ = true;
	return true;
}

void PersistentBuffer::cleanup() {
	if (!initialized_) {
		return;
	}

	// Delete fences
	for (size_t i = 0; i < BUFFER_COUNT; ++i) {
		if (fences_[i]) {
			glDeleteSync(fences_[i]);
			fences_[i] = nullptr;
		}
	}

	// Unmap and delete buffer
	if (bufferId_) {
		glUnmapNamedBuffer(bufferId_);
		glDeleteBuffers(1, &bufferId_);
		bufferId_ = 0;
	}

	mappedPtr_ = nullptr;
	initialized_ = false;
}

void* PersistentBuffer::waitAndMap(size_t count) {
	if (!initialized_ || count > maxElements_) {
		return nullptr;
	}

	// Wait for fence on current section if it exists
	if (fences_[currentSection_]) {
		GLenum result = glClientWaitSync(fences_[currentSection_], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000); // 1 sec timeout

		if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
			std::cerr << "PersistentBuffer: Fence wait timeout/failed on section " << currentSection_ << std::endl;
			// Do NOT proceed if GPU is still reading - would cause undefined behavior
			return nullptr;
		}

		// Delete the fence we just waited on
		glDeleteSync(fences_[currentSection_]);
		fences_[currentSection_] = nullptr;
	}

	// Return pointer to current section
	return static_cast<char*>(mappedPtr_) + (currentSection_ * sectionSize_);
}

void PersistentBuffer::signalFinished() {
	if (!initialized_) {
		return;
	}

	// Insert fence for this section
	fences_[currentSection_] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	// Advance to next section
	currentSection_ = (currentSection_ + 1) % BUFFER_COUNT;
}

size_t PersistentBuffer::getCurrentSectionOffset() const {
	return currentSection_ * sectionSize_;
}
