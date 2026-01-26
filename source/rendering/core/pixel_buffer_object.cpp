#include "rendering/core/pixel_buffer_object.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <cstring>
#include <utility>

PixelBufferObject::~PixelBufferObject() {
	cleanup();
}

PixelBufferObject::PixelBufferObject(PixelBufferObject&& other) noexcept
	:
	current_index_(other.current_index_),
	size_(other.size_), initialized_(other.initialized_) {
	for (int i = 0; i < BUFFER_COUNT; ++i) {
		buffers_[i] = std::move(other.buffers_[i]);
		fences_[i] = std::move(other.fences_[i]);
	}
	other.initialized_ = false;
	other.size_ = 0;
}

PixelBufferObject& PixelBufferObject::operator=(PixelBufferObject&& other) noexcept {
	if (this != &other) {
		cleanup();
		current_index_ = other.current_index_;
		size_ = other.size_;
		initialized_ = other.initialized_;

		for (int i = 0; i < BUFFER_COUNT; ++i) {
			buffers_[i] = std::move(other.buffers_[i]);
			fences_[i] = std::move(other.fences_[i]);
		}

		other.initialized_ = false;
		other.size_ = 0;
	}
	return *this;
}

bool PixelBufferObject::initialize(size_t size) {
	if (initialized_) {
		return true;
	}

	size_ = size;

	for (int i = 0; i < BUFFER_COUNT; ++i) {
		buffers_[i] = std::make_unique<GLBuffer>();
		glNamedBufferData(buffers_[i]->GetID(), size, nullptr, GL_STREAM_DRAW);
	}

	current_index_ = 0;
	initialized_ = true;
	return true;
}

void PixelBufferObject::cleanup() {
	if (!initialized_) {
		return;
	}

	for (int i = 0; i < BUFFER_COUNT; ++i) {
		buffers_[i].reset();
		fences_[i].reset();
	}
	initialized_ = false;
}

void* PixelBufferObject::mapWrite() {
	if (!initialized_) {
		return nullptr;
	}

	// Wait for GPU to finish reading from this PBO (if it was used previously)
	if (fences_[current_index_]) {
		GLenum result = fences_[current_index_].clientWait(GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
		if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
			spdlog::error("PixelBufferObject: Fence wait failed");
			return nullptr;
		}
		fences_[current_index_].reset();
	}

	// Map buffer (invalidate old data to avoid stalls)
	return glMapNamedBufferRange(buffers_[current_index_]->GetID(), 0, size_, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void PixelBufferObject::unmap() {
	glUnmapNamedBuffer(buffers_[current_index_]->GetID());
}

void PixelBufferObject::bind() {
	if (initialized_) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffers_[current_index_]->GetID());
	}
}

void PixelBufferObject::unbind() {
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void PixelBufferObject::advance() {
	if (!initialized_) {
		return;
	}

	// Place fence to mark that GPU is about to read from this PBO (the texture upload command)
	fences_[current_index_].reset(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

	current_index_ = (current_index_ + 1) % BUFFER_COUNT;
}
