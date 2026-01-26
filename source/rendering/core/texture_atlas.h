#ifndef RME_RENDERING_CORE_TEXTURE_ATLAS_H_
#define RME_RENDERING_CORE_TEXTURE_ATLAS_H_

#include "app/main.h"
#include "rendering/core/pixel_buffer_object.h"
#include "rendering/core/gl_resources.h"
#include <optional>
#include <cstdint>
#include <memory>

/**
 * AtlasRegion represents where a sprite is located in the texture atlas.
 * Contains UV coordinates and layer index for sampling.
 */
struct AtlasRegion {
	uint32_t atlas_index = 0; // Layer in texture array
	float u_min = 0.0f; // UV left
	float v_min = 0.0f; // UV top
	float u_max = 1.0f; // UV right
	float v_max = 1.0f; // UV bottom
};

/**
 * TextureAtlas manages a GL_TEXTURE_2D_ARRAY that packs many sprites per layer.
 *
 * Each layer is ATLAS_SIZE x ATLAS_SIZE (4096x4096).
 * Each sprite is SPRITE_SIZE x SPRITE_SIZE (32x32).
 * This gives SPRITES_PER_ROW (128) sprites per row, and SPRITES_PER_LAYER (16384) per layer.
 *
 * Based on the imgui_renderer_example_readonly reference implementation.
 */
class TextureAtlas {
public:
	static constexpr int ATLAS_SIZE = 4096;
	static constexpr int SPRITE_SIZE = 32;
	static constexpr int SPRITES_PER_ROW = ATLAS_SIZE / SPRITE_SIZE; // 128
	static constexpr int SPRITES_PER_LAYER = SPRITES_PER_ROW * SPRITES_PER_ROW; // 16384
	static constexpr int MAX_LAYERS = 64; // 64 * 16384 = 1M+ sprites

	TextureAtlas();
	~TextureAtlas();

	// Non-copyable
	TextureAtlas(const TextureAtlas&) = delete;
	TextureAtlas& operator=(const TextureAtlas&) = delete;

	// Moveable
	TextureAtlas(TextureAtlas&& other) noexcept;
	TextureAtlas& operator=(TextureAtlas&& other) noexcept;

	/**
	 * Initialize the texture array.
	 * @param initial_layers Number of layers to pre-allocate (default: 8)
	 * @return true if successful
	 */
	bool initialize(int initial_layers = 8);

	/**
	 * Add a 32x32 sprite to the atlas.
	 * @param rgba_data Pointer to 32*32*4 bytes of RGBA pixel data
	 * @return AtlasRegion with layer and UV coordinates, or nullopt on failure
	 */
	std::optional<AtlasRegion> addSprite(const uint8_t* rgba_data);

	/**
	 * Bind the texture array to a texture slot.
	 */
	void bind(uint32_t slot = 0) const;

	/**
	 * Unbind texture.
	 */
	void unbind() const;

	/**
	 * Get layer count.
	 */
	int getLayerCount() const {
		return layer_count_;
	}

	/**
	 * Get OpenGL texture ID.
	 */
	GLuint id() const {
		return texture_ ? texture_->GetID() : 0;
	}

	/**
	 * Check if atlas is valid.
	 */
	bool isValid() const {
		return texture_ != nullptr;
	}

	/**
	 * Release resources.
	 */
	void release();

private:
	bool addLayer();

	// PBO for async uploads
	std::unique_ptr<PixelBufferObject> pbo_;

	std::unique_ptr<GLTextureResource> texture_;
	int layer_count_ = 0;
	int allocated_layers_ = 0;
	int total_sprite_count_ = 0;
	int current_layer_ = 0;
	int next_x_ = 0; // Next slot X in grid
	int next_y_ = 0; // Next slot Y in grid
};

#endif
