#ifndef RME_RENDERING_CORE_SPRITE_BATCH_H_
#define RME_RENDERING_CORE_SPRITE_BATCH_H_

#include "app/main.h"
#include "rendering/core/ring_buffer.h"
#include "rendering/core/multi_draw_indirect_renderer.h"
#include "rendering/core/sprite_instance.h"
#include "rendering/core/shader_program.h"
#include "rendering/core/texture_atlas.h"
#include "rendering/core/atlas_manager.h"
#include "rendering/core/gl_resources.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

/**
 * High-performance batched sprite renderer using instanced drawing.
 *
 * Features:
 * - Instanced rendering (1 quad shared + 64-byte instance data)
 * - Triple-buffered persistent mapping (RingBuffer)
 * - Multi-Draw Indirect support (GL 4.3+)
 * - Single draw call per batch (with MDI) or per texture (without MDI)
 */
class SpriteBatch {
public:
	// 100k sprites buffer = ~6.4MB
	static constexpr size_t MAX_SPRITES_PER_BATCH = 100000;

	SpriteBatch();
	~SpriteBatch();

	// Non-copyable, but movable
	SpriteBatch(const SpriteBatch&) = delete;
	SpriteBatch& operator=(const SpriteBatch&) = delete;
	SpriteBatch(SpriteBatch&& other) noexcept;
	SpriteBatch& operator=(SpriteBatch&& other) noexcept;

	/**
	 * Initialize GPU resources (shader, VAO, RingBuffer).
	 * @return true if successful
	 */
	bool initialize();

	/**
	 * Begin a new batch. Clears pending sprites.
	 * @param projection The orthographic projection matrix
	 */
	void begin(const glm::mat4& projection);

	/**
	 * Queue a sprite for rendering.
	 * @param x Screen X
	 * @param y Screen Y
	 * @param w Width
	 * @param h Height
	 * @param region Atlas region for UVs
	 */
	void draw(float x, float y, float w, float h, const AtlasRegion& region);

	/**
	 * Queue a sprite with tint.
	 */
	void draw(float x, float y, float w, float h, const AtlasRegion& region, float r, float g, float b, float a);

	/**
	 * Draw a solid rectangle using the white pixel from the atlas.
	 * Requires AtlasManager::getWhitePixel() to be valid.
	 */
	void drawRect(float x, float y, float w, float h, const glm::vec4& color, const AtlasManager& atlas_manager);

	/**
	 * Draw a hollow rectangle (outline) using 4 thin rects.
	 */
	void drawRectLines(float x, float y, float w, float h, const glm::vec4& color, const AtlasManager& atlas_manager);

	/**
	 * End batch. Flushes all sprites to GPU.
	 * @param atlas_manager Atlas manager to bind textures from
	 */
	void end(const AtlasManager& atlas_manager);

	/**
	 * Set global tint for subsequent draws in current batch.
	 */
	void setGlobalTint(float r, float g, float b, float a);

	/**
	 * Ensure capacity in pending vector.
	 */
	void ensureCapacity(size_t capacity);

	int getDrawCallCount() const {
		return draw_call_count_;
	}
	int getSpriteCount() const {
		return sprite_count_;
	}

private:
	void flush(const AtlasManager& atlas_manager);

	std::unique_ptr<ShaderProgram> shader_;

	std::unique_ptr<GLVertexArray> vao_;
	std::unique_ptr<GLBuffer> quad_vbo_;
	std::unique_ptr<GLBuffer> quad_ebo_;

	RingBuffer ring_buffer_;
	MultiDrawIndirectRenderer mdi_renderer_;

	std::vector<SpriteInstance> pending_sprites_;
	glm::mat4 projection_ { 1.0f };
	glm::vec4 global_tint_ { 1.0f };

	bool in_batch_ = false;
	bool use_mdi_ = false;

	int draw_call_count_ = 0;
	int sprite_count_ = 0;
	GLuint last_bound_vao_ = 0;
};

#endif
