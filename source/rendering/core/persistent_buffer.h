#ifndef RME_RENDERING_CORE_PERSISTENT_BUFFER_H_
#define RME_RENDERING_CORE_PERSISTENT_BUFFER_H_

#include "main.h"

/**
 * Triple-buffered ring buffer with persistent mapping for zero-copy GPU uploads.
 *
 * Based on the proven imgui_renderer implementation:
 * 1. Maps buffer ONCE at initialization (persistent mapping)
 * 2. Uses 3 sections that rotate after each draw (triple buffering)
 * 3. Uses fence sync to ensure GPU is done with a section before reusing
 *
 * Key insight: Each flush gets its OWN section. No accumulating within sections.
 */
class PersistentBuffer {
public:
	static constexpr size_t BUFFER_COUNT = 3;

	PersistentBuffer() = default;
	~PersistentBuffer();

	// Non-copyable
	PersistentBuffer(const PersistentBuffer&) = delete;
	PersistentBuffer& operator=(const PersistentBuffer&) = delete;

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
	 * Signal that we've finished drawing. Call AFTER each draw call!
	 * Inserts fence and advances to next section.
	 */
	void signalFinished();

	/**
	 * Get the OpenGL buffer ID.
	 */
	GLuint getBufferId() const {
		return bufferId_;
	}

	/**
	 * Get byte offset to current section for vertex attribute setup.
	 */
	size_t getCurrentSectionOffset() const;

	/**
	 * Get max elements per section.
	 */
	size_t getMaxElements() const {
		return maxElements_;
	}

	/**
	 * Get element size in bytes.
	 */
	size_t getElementSize() const {
		return elementSize_;
	}

private:
	GLuint bufferId_ = 0;
	void* mappedPtr_ = nullptr;
	GLsync fences_[BUFFER_COUNT] = { nullptr, nullptr, nullptr };

	size_t elementSize_ = 0;
	size_t maxElements_ = 0;
	size_t sectionSize_ = 0; // elementSize * maxElements
	size_t currentSection_ = 0;

	bool initialized_ = false;
};

#endif
