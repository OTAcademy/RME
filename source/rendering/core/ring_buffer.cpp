#include "rendering/core/ring_buffer.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <cstring>
#include <utility>

RingBuffer::~RingBuffer() {
	cleanup();
}

RingBuffer::RingBuffer(RingBuffer&& other) noexcept
	:
	buffer_id_(other.buffer_id_),
	mapped_ptr_(other.mapped_ptr_), element_size_(other.element_size_), max_elements_(other.max_elements_), section_size_(other.section_size_), current_section_(other.current_section_), use_persistent_mapping_(other.use_persistent_mapping_), initialized_(other.initialized_) {
	for (size_t i = 0; i < BUFFER_COUNT; ++i) {
		fences_[i] = std::move(other.fences_[i]);
	}
	other.buffer_id_ = 0;
	other.mapped_ptr_ = nullptr;
	other.initialized_ = false;
}

RingBuffer& RingBuffer::operator=(RingBuffer&& other) noexcept {
	if (this != &other) {
		cleanup();
		buffer_id_ = other.buffer_id_;
		mapped_ptr_ = other.mapped_ptr_;
		for (size_t i = 0; i < BUFFER_COUNT; ++i) {
			fences_[i] = std::move(other.fences_[i]);
		}
		element_size_ = other.element_size_;
		max_elements_ = other.max_elements_;
		section_size_ = other.section_size_;
		current_section_ = other.current_section_;
		use_persistent_mapping_ = other.use_persistent_mapping_;
		initialized_ = other.initialized_;

		other.buffer_id_ = 0;
		other.mapped_ptr_ = nullptr;
		other.initialized_ = false;
	}
	return *this;
}

bool RingBuffer::initialize(size_t element_size, size_t max_elements) {
	if (initialized_) {
		return true;
	}

	element_size_ = element_size;
	max_elements_ = max_elements;
	section_size_ = element_size * max_elements;

	// Total buffer size: 3 sections
	size_t total_size = section_size_ * BUFFER_COUNT;

	// Create buffer
	glCreateBuffers(1, &buffer_id_);
	if (buffer_id_ == 0) {
		spdlog::error("RingBuffer: Failed to create buffer");
		return false;
	}

	// GL 4.4+ persistent coherent mapping
	GLbitfield storage_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	// Use NamedBufferStorage if available (GL 4.5+) or bind and use BufferStorage
	// RME uses GL 4.5, so glNamedBufferStorage is safe
	glNamedBufferStorage(buffer_id_, total_size, nullptr, storage_flags);

	GLbitfield map_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	mapped_ptr_ = glMapNamedBufferRange(buffer_id_, 0, total_size, map_flags);

	if (!mapped_ptr_) {
		spdlog::error("RingBuffer: Persistent mapping failed");
		glDeleteBuffers(1, &buffer_id_);
		buffer_id_ = 0;
		return false;
	}

	use_persistent_mapping_ = true;
	initialized_ = true;
	return true;
}

void RingBuffer::cleanup() {
	if (!initialized_) {
		return;
	}

	// Reset fences (SyncHandle destructor handles glDeleteSync)
	for (size_t i = 0; i < BUFFER_COUNT; ++i) {
		fences_[i].reset();
	}

	// Unmap and delete buffer
	if (buffer_id_) {
		if (mapped_ptr_) {
			glUnmapNamedBuffer(buffer_id_);
			mapped_ptr_ = nullptr;
		}
		glDeleteBuffers(1, &buffer_id_);
		buffer_id_ = 0;
	}

	initialized_ = false;
}

void* RingBuffer::waitAndMap(size_t count) {
	if (!initialized_ || count > max_elements_) {
		return nullptr;
	}

	// Wait for fence on current section if it exists
	if (fences_[current_section_]) {
		// Wait with timeout
		GLenum result = fences_[current_section_].clientWait(
			GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000
		); // 1 second timeout

		if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
			spdlog::error("RingBuffer: Fence wait timeout/failed on section {}", current_section_);
			// Return nullptr to avoid UB (writing while GPU reads)
			return nullptr;
		}

		// Reset the fence
		fences_[current_section_].reset();
	}

	// Return pointer to current section
	return static_cast<char*>(mapped_ptr_) + (current_section_ * section_size_);
}

void RingBuffer::finishWrite() {
	// Persistent mapping: buffer stays mapped, nothing to unmap
}

void RingBuffer::signalFinished() {
	if (!initialized_) {
		return;
	}

	// Insert fence for this section
	fences_[current_section_].reset(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

	// Advance to next section
	current_section_ = (current_section_ + 1) % BUFFER_COUNT;
}

void RingBuffer::advance() {
	if (!initialized_) {
		return;
	}
	current_section_ = (current_section_ + 1) % BUFFER_COUNT;
}

void RingBuffer::fence(size_t section_index) {
	if (!initialized_ || section_index >= BUFFER_COUNT) {
		return;
	}
	fences_[section_index].reset(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

size_t RingBuffer::getCurrentSectionOffset() const {
	return current_section_ * section_size_;
}
