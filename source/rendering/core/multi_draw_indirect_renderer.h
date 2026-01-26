#ifndef RME_RENDERING_CORE_MULTI_DRAW_INDIRECT_RENDERER_H_
#define RME_RENDERING_CORE_MULTI_DRAW_INDIRECT_RENDERER_H_

#include "app/main.h"
#include "rendering/core/gl_resources.h"
#include <vector>
#include <cstdint>
#include <memory>

/**
 * GPU command buffer for glMultiDrawElementsIndirect (GL 4.3+).
 *
 * Reduces 2-5 draw calls (one per atlas) to ONE draw call total.
 * GPU executes command buffer directly, eliminating CPU→GPU sync per draw.
 */
class MultiDrawIndirectRenderer {
public:
	/**
	 * OpenGL indirect draw command structure (must match spec exactly).
	 */
	struct DrawElementsIndirectCommand {
		GLuint count; // Number of indices per instance (6 for quad)
		GLuint instanceCount; // Number of instances (sprites in this batch)
		GLuint firstIndex; // Offset into EBO
		GLuint baseVertex; // Offset into VBO
		GLuint baseInstance; // Offset into instance buffer
	};

	static constexpr int MAX_COMMANDS = 16; // Support up to 16 atlases

	MultiDrawIndirectRenderer();
	~MultiDrawIndirectRenderer();

	// Non-copyable, but movable
	MultiDrawIndirectRenderer(const MultiDrawIndirectRenderer&) = delete;
	MultiDrawIndirectRenderer& operator=(const MultiDrawIndirectRenderer&) = delete;

	MultiDrawIndirectRenderer(MultiDrawIndirectRenderer&& other) noexcept;
	MultiDrawIndirectRenderer& operator=(MultiDrawIndirectRenderer&& other) noexcept;

	/**
	 * Initialize GPU buffer for indirect commands.
	 * @return true if successful (requires GL 4.3+)
	 */
	bool initialize();

	/**
	 * Cleanup GPU resources.
	 */
	void cleanup();

	/**
	 * Clear command buffer for new frame.
	 */
	void clear();

	/**
	 * Add a draw command to the buffer.
	 * @param count Indices per instance (usually 6 for quad)
	 * @param instanceCount Number of sprites in this batch
	 * @param firstIndex Offset into index buffer
	 * @param baseVertex Offset into vertex buffer
	 * @param baseInstance Offset into instance data
	 */
	void addDrawCommand(GLuint count, GLuint instanceCount, GLuint firstIndex = 0, GLuint baseVertex = 0, GLuint baseInstance = 0);

	/**
	 * Upload command buffer to GPU.
	 */
	void upload();

	/**
	 * Execute all draw commands with single glMultiDrawElementsIndirect call.
	 * VAO and shader must already be bound.
	 */
	void execute();

	/**
	 * Get number of pending commands.
	 */
	size_t getCommandCount() const {
		return commands_.size();
	}

	/**
	 * Check if MDI is available (GL 4.3+).
	 */
	bool isAvailable() const {
		return available_;
	}

private:
	std::vector<DrawElementsIndirectCommand> commands_;
	std::unique_ptr<GLBuffer> command_buffer_;
	bool available_ = false;
	bool initialized_ = false;
};

#endif
