#include "rendering/core/texture_atlas.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>

TextureAtlas::TextureAtlas() = default;

TextureAtlas::~TextureAtlas() {
	release();
}

TextureAtlas::TextureAtlas(TextureAtlas&& other) noexcept
	:
	texture_id_(other.texture_id_),
	layer_count_(other.layer_count_),
	allocated_layers_(other.allocated_layers_),
	total_sprite_count_(other.total_sprite_count_),
	current_layer_(other.current_layer_), next_x_(other.next_x_),
	next_y_(other.next_y_) {
	other.texture_id_ = 0;
	other.layer_count_ = 0;
	other.allocated_layers_ = 0;
	other.total_sprite_count_ = 0;
	other.current_layer_ = 0;
	other.next_x_ = 0;
	other.next_y_ = 0;
}

TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other) noexcept {
	if (this != &other) {
		release();
		texture_id_ = other.texture_id_;
		layer_count_ = other.layer_count_;
		allocated_layers_ = other.allocated_layers_;
		total_sprite_count_ = other.total_sprite_count_;
		current_layer_ = other.current_layer_;
		next_x_ = other.next_x_;
		next_y_ = other.next_y_;
		other.texture_id_ = 0;
		other.layer_count_ = 0;
		other.allocated_layers_ = 0;
		other.total_sprite_count_ = 0;
		other.current_layer_ = 0;
		other.next_x_ = 0;
		other.next_y_ = 0;
	}
	return *this;
}

bool TextureAtlas::initialize(int initial_layers) {
	if (texture_id_ != 0) {
		return true; // Already initialized
	}

	if (initial_layers < 1) {
		initial_layers = 1;
	}
	if (initial_layers > MAX_LAYERS) {
		initial_layers = MAX_LAYERS;
	}

	glGenTextures(1, &texture_id_);
	if (texture_id_ == 0) {
		spdlog::error("TextureAtlas: Failed to generate texture");
		return false;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Allocate texture array storage
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, initial_layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	allocated_layers_ = initial_layers;
	layer_count_ = 1; // Start with one active layer
	current_layer_ = 0;
	next_x_ = 0;
	next_y_ = 0;

	// Initialize PBO
	pbo_ = std::make_unique<PixelBufferObject>();
	if (!pbo_->initialize(SPRITE_SIZE * SPRITE_SIZE * 4)) {
		spdlog::error("TextureAtlas: Failed to initialize PBO");
		return false;
	}

	spdlog::info("TextureAtlas created: {}x{} x {} layers, id={}", ATLAS_SIZE, ATLAS_SIZE, initial_layers, texture_id_);
	return true;
}

bool TextureAtlas::addLayer() {
	if (layer_count_ >= MAX_LAYERS) {
		spdlog::error("TextureAtlas: Max layers ({}) reached", MAX_LAYERS);
		return false;
	}

	// If we need more layers than allocated, reallocate
	if (layer_count_ >= allocated_layers_) {
		// Linear growth to prevent massive VRAM spikes
		// 4 layers = ~268 MB VRAM
		int new_allocated = std::min(allocated_layers_ + 4, MAX_LAYERS);

		spdlog::info("TextureAtlas: Expanding {} -> {} layers", allocated_layers_, new_allocated);

		// Create new larger texture array
		GLuint new_texture;
		glGenTextures(1, &new_texture);
		if (new_texture == 0) {
			spdlog::error("TextureAtlas: Failed to generate new texture id during expansion");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, new_texture);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, new_allocated, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			spdlog::error("TextureAtlas: glTexImage3D failed during expansion (err={}). VRAM might be full.", err);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glDeleteTextures(1, &new_texture);
			return false;
		}

		// Copy existing layers using glCopyImageSubData (GL 4.3+)
		glCopyImageSubData(texture_id_, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, new_texture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, ATLAS_SIZE, ATLAS_SIZE, allocated_layers_);

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		// Removed glFinish() - it causes main thread to freeze waiting for GPU
		// The driver should handle synchronization implicitly for the next draw call

		glDeleteTextures(1, &texture_id_);
		texture_id_ = new_texture;
		allocated_layers_ = new_allocated;
	}

	layer_count_++;
	current_layer_ = layer_count_ - 1;
	next_x_ = 0;
	next_y_ = 0;

	return true;
}

std::optional<AtlasRegion> TextureAtlas::addSprite(const uint8_t* rgba_data) {
	if (!isValid()) {
		std::cerr << "TextureAtlas::addSprite called on uninitialized atlas" << std::endl;
		return std::nullopt;
	}

	if (!rgba_data) {
		std::cerr << "TextureAtlas::addSprite called with null data" << std::endl;
		return std::nullopt;
	}

	// Check if current layer is full
	if (next_y_ >= SPRITES_PER_ROW) {
		if (!addLayer()) {
			return std::nullopt;
		}
	}

	// Calculate pixel position in current layer
	int pixel_x = next_x_ * SPRITE_SIZE;
	int pixel_y = next_y_ * SPRITE_SIZE;

	// Upload sprite data to texture array
	bool uploaded = false;
	if (pbo_) {
		void* ptr = pbo_->mapWrite();
		if (ptr) {
			memcpy(ptr, rgba_data, SPRITE_SIZE * SPRITE_SIZE * 4);
			pbo_->unmap();

			pbo_->bind(); // Binds GL_PIXEL_UNPACK_BUFFER

			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
			// Offset is 0 in PBO
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, pixel_x, pixel_y, current_layer_, SPRITE_SIZE, SPRITE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			pbo_->unbind();
			pbo_->advance();
			uploaded = true;
		}
	}

	if (!uploaded) {
		// Fallback synchronously
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, pixel_x, pixel_y, current_layer_, SPRITE_SIZE, SPRITE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	// Calculate UV coordinates with half-texel inset to prevent bleeding
	const float texel_size = 1.0f / static_cast<float>(ATLAS_SIZE);
	const float half_texel = texel_size * 0.5f;

	AtlasRegion region;
	region.atlas_index = static_cast<uint32_t>(current_layer_);
	region.u_min = static_cast<float>(pixel_x) / ATLAS_SIZE + half_texel;
	region.v_min = static_cast<float>(pixel_y) / ATLAS_SIZE + half_texel;
	region.u_max = static_cast<float>(pixel_x + SPRITE_SIZE) / ATLAS_SIZE - half_texel;
	region.v_max = static_cast<float>(pixel_y + SPRITE_SIZE) / ATLAS_SIZE - half_texel;

	// Advance to next slot
	next_x_++;
	if (next_x_ >= SPRITES_PER_ROW) {
		next_x_ = 0;
		next_y_++;
	}
	total_sprite_count_++;

	return region;
}

void TextureAtlas::bind(uint32_t slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
}

void TextureAtlas::unbind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureAtlas::release() {
	if (texture_id_ != 0) {
		glDeleteTextures(1, &texture_id_);
		texture_id_ = 0;
	}
	layer_count_ = 0;
	allocated_layers_ = 0;
	total_sprite_count_ = 0;
	current_layer_ = 0;
	next_x_ = 0;
	next_y_ = 0;
}
