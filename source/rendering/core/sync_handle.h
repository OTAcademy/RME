#ifndef RME_RENDERING_CORE_SYNC_HANDLE_H_
#define RME_RENDERING_CORE_SYNC_HANDLE_H_

#include "app/main.h"

#include <utility>

/**
 * RAII wrapper for OpenGL fence sync objects.
 *
 * Provides automatic cleanup of GLsync handles.
 * Supports move semantics but not copy semantics.
 *
 * Usage:
 *   SyncHandle fence;
 *   fence.reset(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
 *   // ... later ...
 *   fence.clientWait(GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
 */
class SyncHandle {
public:
	/**
	 * Default constructor - no fence.
	 */
	SyncHandle() = default;

	/**
	 * Construct from existing sync object (takes ownership).
	 */
	explicit SyncHandle(GLsync sync) :
		sync_(sync) { }

	/**
	 * Destructor - deletes the sync object if valid.
	 */
	~SyncHandle() {
		if (sync_) {
			glDeleteSync(sync_);
		}
	}

	/**
	 * Move constructor - transfers ownership.
	 */
	SyncHandle(SyncHandle&& other) noexcept :
		sync_(other.sync_) {
		other.sync_ = nullptr;
	}

	/**
	 * Move assignment - releases current and transfers ownership.
	 */
	SyncHandle& operator=(SyncHandle&& other) noexcept {
		if (this != &other) {
			if (sync_) {
				glDeleteSync(sync_);
			}
			sync_ = other.sync_;
			other.sync_ = nullptr;
		}
		return *this;
	}

	// No copy semantics
	SyncHandle(const SyncHandle&) = delete;
	SyncHandle& operator=(const SyncHandle&) = delete;

	/**
	 * Get the raw GLsync handle.
	 */
	GLsync get() const {
		return sync_;
	}

	/**
	 * Reset to a new sync object (deletes old one if present).
	 */
	void reset(GLsync new_sync = nullptr) {
		if (sync_) {
			glDeleteSync(sync_);
		}
		sync_ = new_sync;
	}

	/**
	 * Release ownership without deleting.
	 */
	GLsync release() {
		GLsync tmp = sync_;
		sync_ = nullptr;
		return tmp;
	}

	/**
	 * Check if handle is valid.
	 */
	explicit operator bool() const {
		return sync_ != nullptr;
	}

	/**
	 * Check if handle is valid.
	 */
	bool isValid() const {
		return sync_ != nullptr;
	}

	/**
	 * Wait for the sync object to be signaled.
	 *
	 * @param flags Typically GL_SYNC_FLUSH_COMMANDS_BIT
	 * @param timeout_ns Timeout in nanoseconds
	 * @return GL_ALREADY_SIGNALED, GL_TIMEOUT_EXPIRED, GL_CONDITION_SATISFIED, or GL_WAIT_FAILED
	 */
	GLenum clientWait(GLbitfield flags, GLuint64 timeout_ns) {
		if (!sync_) {
			return GL_CONDITION_SATISFIED;
		}
		return glClientWaitSync(sync_, flags, timeout_ns);
	}

	/**
	 * Non-blocking check if signaled.
	 */
	bool isSignaled() {
		if (!sync_) {
			return true;
		}
		GLenum result = glClientWaitSync(sync_, 0, 0);
		return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
	}

private:
	GLsync sync_ = nullptr;
};

#endif
