#ifndef RME_RENDERING_CORE_GL_SCOPED_STATE_H_
#define RME_RENDERING_CORE_GL_SCOPED_STATE_H_

#include <glad/glad.h>

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

#endif
