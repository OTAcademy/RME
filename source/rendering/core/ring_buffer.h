#ifndef RME_RENDERING_CORE_RING_BUFFER_H_
#define RME_RENDERING_CORE_RING_BUFFER_H_

#include "app/main.h"
#include "rendering/core/sync_handle.h"
#include <cstddef>
#include <cstdint>

/**
 * Triple-buffered ring buffer with persistent mapping for zero-copy GPU uploads.
 *
 * This eliminates CPU-GPU synchronization stalls by:
 * 1. Mapping the buffer ONCE at initialization (persistent mapping)
 * 2. Using 3 sections that rotate each frame (triple buffering)
 * 3. Using fence sync to ensure GPU is done with a section before reusing
 *
 * Requires OpenGL 4.4+.
 */
class RingBuffer {
public:
	static constexpr size_t BUFFER_COUNT = 3;

	RingBuffer() = default;
	~RingBuffer();

	// Non-copyable
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator=(const RingBuffer&) = delete;

	// Movable
	RingBuffer(RingBuffer&& other) noexcept;
	RingBuffer& operator=(RingBuffer&& other) noexcept;

	/**
	 * Initialize the ring buffer with persistent mapping.
	 * @param element_size Size of each element in bytes
	 * @param max_elements Maximum elements per section
	 * @return true if successful
	 */
	bool initialize(size_t element_size, size_t max_elements);

	/**
	 * Release GPU resources.
	 */
	void cleanup();

	/**
	 * Wait for the current section to be available, return write pointer.
	 * This will block if the GPU is still reading from this section.
	 * @param count Number of elements to write (for bounds checking)
	 * @return Pointer to write data, or nullptr if count exceeds capacity
	 */
	void* waitAndMap(size_t count);

	/**
	 * Signal that we've finished writing.
	 * For persistent mapping, this is a no-op but kept for API compatibility.
	 */
	void finishWrite();

	/**
	 * Signal that we've finished drawing. Call after draw calls!
	 * Inserts fence and advances to next section.
	 */
	void signalFinished();

	size_t getCapacity() const {
		return max_elements_;
	}
	size_t getCurrentSectionIndex() const {
		return current_section_;
	}
	void advance();
	void fence(size_t section_index);

	/**
	 * Get the OpenGL buffer ID.
	 */
	GLuint getBufferId() const {
		return buffer_id_;
	}

	/**
	 * Get byte offset to current section for vertex attribute setup.
	 */
	size_t getCurrentSectionOffset() const;

	/**
	 * Get max elements per section.
	 */
	size_t getMaxElements() const {
		return max_elements_;
	}

	/**
	 * Check if using persistent mapping (always true for this implementation).
	 */
	bool isPersistentlyMapped() const {
		return use_persistent_mapping_;
	}

private:
	GLuint buffer_id_ = 0;
	void* mapped_ptr_ = nullptr;
	SyncHandle fences_[BUFFER_COUNT];

	size_t element_size_ = 0;
	size_t max_elements_ = 0;
	size_t section_size_ = 0; // element_size * max_elements
	size_t current_section_ = 0;

	bool use_persistent_mapping_ = false;
	bool initialized_ = false;
};

#endif
