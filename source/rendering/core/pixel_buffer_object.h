#ifndef RME_RENDERING_CORE_PIXEL_BUFFER_OBJECT_H_
#define RME_RENDERING_CORE_PIXEL_BUFFER_OBJECT_H_

#include "app/main.h"
#include "rendering/core/sync_handle.h"
#include "rendering/core/gl_resources.h"
#include <vector>
#include <memory>
#include <array>

/**
 * Manages Pixel Buffer Objects (PBOs) for asynchronous texture uploads.
 *
 * Uses double-buffering (2 PBOs) to allow CPU to write to one buffer
 * while GPU reads from the other via DMA, preventing stalls.
 */
class PixelBufferObject {
public:
	static constexpr int BUFFER_COUNT = 2;

	PixelBufferObject() = default;
	~PixelBufferObject();

	// Non-copyable
	PixelBufferObject(const PixelBufferObject&) = delete;
	PixelBufferObject& operator=(const PixelBufferObject&) = delete;

	// Movable
	PixelBufferObject(PixelBufferObject&& other) noexcept;
	PixelBufferObject& operator=(PixelBufferObject&& other) noexcept;

	/**
	 * Initialize PBOs.
	 * @param size Size of the buffer in bytes (e.g. 32x32x4 for a sprite)
	 * @return true if successful
	 */
	bool initialize(size_t size);

	/**
	 * Release GPU resources.
	 */
	void cleanup();

	/**
	 * Map the current PBO for writing.
	 * Waits for unrelated GPU operations if necessary.
	 * @return Pointer to mapped memory
	 */
	void* mapWrite();

	/**
	 * Unmap the PBO.
	 */
	void unmap();

	/**
	 * Bind the PBO as GL_PIXEL_UNPACK_BUFFER.
	 */
	void bind();

	/**
	 * Unbind GL_PIXEL_UNPACK_BUFFER.
	 */
	void unbind();

	/**
	 * Advance to next buffer and place fence.
	 * Call this AFTER glTexSubImage2D/3D.
	 */
	void advance();

	size_t getSize() const {
		return size_;
	}

private:
	std::array<std::unique_ptr<GLBuffer>, BUFFER_COUNT> buffers_;
	std::array<SyncHandle, BUFFER_COUNT> fences_;

	size_t size_ = 0;
	int current_index_ = 0;
	bool initialized_ = false;
};

#endif
