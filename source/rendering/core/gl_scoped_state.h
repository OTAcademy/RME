#ifndef RME_RENDERING_CORE_GL_SCOPED_STATE_H_
#define RME_RENDERING_CORE_GL_SCOPED_STATE_H_

#include <glad/glad.h>
#include <cassert>

/**
 * @brief RAII wrapper for glEnable/glDisable
 *
 * Intended for short-lived scope-based state changes, not for persistent state modifications.
 */
class ScopedGLCapability {
public:
	[[nodiscard]] explicit ScopedGLCapability(GLenum capability, bool enable = true) :
		capability_(capability) {
		was_enabled_ = glIsEnabled(capability);
		if (enable) {
			if (!was_enabled_) {
				glEnable(capability);
			}
		} else {
			if (was_enabled_) {
				glDisable(capability);
			}
		}
	}

	~ScopedGLCapability() {
		if (was_enabled_) {
			glEnable(capability_);
		} else {
			glDisable(capability_);
		}
	}

	ScopedGLCapability(const ScopedGLCapability&) = delete;
	ScopedGLCapability& operator=(const ScopedGLCapability&) = delete;

private:
	GLenum capability_;
	GLboolean was_enabled_;
};

/**
 * @brief RAII wrapper for glBlendFunc and glBlendEquation
 *
 * Intended for short-lived scope-based state changes, not for persistent state modifications.
 */
class ScopedGLBlend {
public:
	[[nodiscard]] ScopedGLBlend(GLenum sfactor, GLenum dfactor) {
		save_state();
		glBlendFunc(sfactor, dfactor);
	}

	[[nodiscard]] ScopedGLBlend(GLenum sfactor, GLenum dfactor, GLenum equation) {
		save_state();
		glBlendFunc(sfactor, dfactor);
		glBlendEquation(equation);
	}

	~ScopedGLBlend() {
		glBlendFuncSeparate(prev_src_rgb_, prev_dst_rgb_, prev_src_alpha_, prev_dst_alpha_);
		glBlendEquationSeparate(prev_eq_rgb_, prev_eq_alpha_);
	}

	ScopedGLBlend(const ScopedGLBlend&) = delete;
	ScopedGLBlend& operator=(const ScopedGLBlend&) = delete;

private:
	void save_state() {
		glGetIntegerv(GL_BLEND_SRC_RGB, &prev_src_rgb_);
		glGetIntegerv(GL_BLEND_DST_RGB, &prev_dst_rgb_);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &prev_src_alpha_);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &prev_dst_alpha_);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &prev_eq_rgb_);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &prev_eq_alpha_);
	}

	GLint prev_src_rgb_, prev_dst_rgb_;
	GLint prev_src_alpha_, prev_dst_alpha_;
	GLint prev_eq_rgb_, prev_eq_alpha_;
};

/**
 * @brief RAII wrapper for glFramebuffer
 *
 * Saves current READ and DRAW framebuffer bindings on construction and restores them on destruction.
 */
class ScopedGLFramebuffer {
public:
	[[nodiscard]] explicit ScopedGLFramebuffer(GLenum target, GLuint framebuffer) :
		target_(target) {
		assert(target_ == GL_FRAMEBUFFER || target_ == GL_READ_FRAMEBUFFER || target_ == GL_DRAW_FRAMEBUFFER);

		if (target_ == GL_FRAMEBUFFER || target_ == GL_READ_FRAMEBUFFER) {
			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_);
		}
		if (target_ == GL_FRAMEBUFFER || target_ == GL_DRAW_FRAMEBUFFER) {
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_draw_);
		}

		glBindFramebuffer(target_, framebuffer);
	}

	~ScopedGLFramebuffer() {
		if (target_ == GL_FRAMEBUFFER || target_ == GL_READ_FRAMEBUFFER) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read_);
		}
		if (target_ == GL_FRAMEBUFFER || target_ == GL_DRAW_FRAMEBUFFER) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_draw_);
		}
	}

	ScopedGLFramebuffer(const ScopedGLFramebuffer&) = delete;
	ScopedGLFramebuffer& operator=(const ScopedGLFramebuffer&) = delete;

private:
	GLenum target_;
	GLint prev_read_ = 0;
	GLint prev_draw_ = 0;
};

/**
 * @brief RAII wrapper for glViewport
 *
 * Saves current viewport on construction and restores it on destruction.
 */
class ScopedGLViewport {
public:
	[[nodiscard]] ScopedGLViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
		glGetIntegerv(GL_VIEWPORT, prev_viewport_);
		glViewport(x, y, width, height);
	}

	~ScopedGLViewport() {
		glViewport(prev_viewport_[0], prev_viewport_[1], prev_viewport_[2], prev_viewport_[3]);
	}

	ScopedGLViewport(const ScopedGLViewport&) = delete;
	ScopedGLViewport& operator=(const ScopedGLViewport&) = delete;

private:
	GLint prev_viewport_[4];
};

#endif
