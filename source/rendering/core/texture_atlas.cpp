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
	texture_id_(std::move(other.texture_id_)),
	layer_count_(other.layer_count_),
	allocated_layers_(other.allocated_layers_),
	total_sprite_count_(other.total_sprite_count_),
	current_layer_(other.current_layer_), next_x_(other.next_x_),
	next_y_(other.next_y_),
	free_slots_(std::move(other.free_slots_)) {
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
		texture_id_ = std::move(other.texture_id_);
		layer_count_ = other.layer_count_;
		allocated_layers_ = other.allocated_layers_;
		total_sprite_count_ = other.total_sprite_count_;
		current_layer_ = other.current_layer_;
		next_x_ = other.next_x_;
		next_y_ = other.next_y_;
		free_slots_ = std::move(other.free_slots_);
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
	if (texture_id_) {
		return true; // Already initialized
	}

	if (initial_layers < 1) {
		initial_layers = 1;
	}
	if (initial_layers > MAX_LAYERS) {
		initial_layers = MAX_LAYERS;
	}

	texture_id_ = std::make_unique<GLTextureResource>(GL_TEXTURE_2D_ARRAY);

	// Set texture parameters
	glTextureParameteri(texture_id_->GetID(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture_id_->GetID(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture_id_->GetID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture_id_->GetID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Allocate texture array storage (Immutable)
	glTextureStorage3D(texture_id_->GetID(), 1, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, initial_layers);

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

	spdlog::info("TextureAtlas created: {}x{} x {} layers, id={}", ATLAS_SIZE, ATLAS_SIZE, initial_layers, texture_id_->GetID());
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
		auto new_texture = std::make_unique<GLTextureResource>(GL_TEXTURE_2D_ARRAY);

		glTextureParameteri(new_texture->GetID(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(new_texture->GetID(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(new_texture->GetID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(new_texture->GetID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTextureStorage3D(new_texture->GetID(), 1, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, new_allocated);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			spdlog::error("TextureAtlas: glTextureStorage3D failed during expansion (err={}). VRAM might be full.", err);
			return false;
		}

		// Copy existing layers using glCopyImageSubData (GL 4.3+)
		glCopyImageSubData(texture_id_->GetID(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, new_texture->GetID(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, ATLAS_SIZE, ATLAS_SIZE, allocated_layers_);

		// Removed glFinish() - it causes main thread to freeze waiting for GPU
		// The driver should handle synchronization implicitly for the next draw call

		texture_id_ = std::move(new_texture);
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
		spdlog::error("TextureAtlas::addSprite called on uninitialized atlas");
		return std::nullopt;
	}

	if (!rgba_data) {
		spdlog::error("TextureAtlas::addSprite called with null data");
		return std::nullopt;
	}

	int pixel_x, pixel_y, layer;
	AtlasRegion region;

	// Check free list first
	if (!free_slots_.empty()) {
		region = free_slots_.back();
		free_slots_.pop_back();

		// Recalculate pixel coords from UV
		// u_min = pixel_x / SIZE + half_texel
		// pixel_x = (u_min - half_texel) * SIZE
		const float texel_size = 1.0f / static_cast<float>(ATLAS_SIZE);
		const float half_texel = texel_size * 0.5f;

		pixel_x = static_cast<int>((region.u_min - half_texel) * ATLAS_SIZE + 0.5f);
		pixel_y = static_cast<int>((region.v_min - half_texel) * ATLAS_SIZE + 0.5f);
		layer = region.atlas_index;
	} else {
		// Check if current layer is full
		if (next_y_ >= SPRITES_PER_ROW) {
			if (!addLayer()) {
				return std::nullopt;
			}
		}

		// Calculate pixel position in current layer
		pixel_x = next_x_ * SPRITE_SIZE;
		pixel_y = next_y_ * SPRITE_SIZE;
		layer = current_layer_;

		// Advance to next slot
		next_x_++;
		if (next_x_ >= SPRITES_PER_ROW) {
			next_x_ = 0;
			next_y_++;
		}
		total_sprite_count_++;
	}

	// Upload sprite data to texture array
	bool uploaded = false;
	if (pbo_) {
		void* ptr = pbo_->mapWrite();
		if (ptr) {
			memcpy(ptr, rgba_data, SPRITE_SIZE * SPRITE_SIZE * 4);
			pbo_->unmap();

			pbo_->bind(); // Binds GL_PIXEL_UNPACK_BUFFER

			// Offset is 0 in PBO
			glTextureSubImage3D(texture_id_->GetID(), 0, pixel_x, pixel_y, layer, SPRITE_SIZE, SPRITE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			pbo_->unbind();
			pbo_->advance();
			uploaded = true;
		}
	}

	if (!uploaded) {
		// Fallback synchronously
		glTextureSubImage3D(texture_id_->GetID(), 0, pixel_x, pixel_y, layer, SPRITE_SIZE, SPRITE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
	}

	// Calculate UV coordinates with half-texel inset to prevent bleeding
	const float texel_size = 1.0f / static_cast<float>(ATLAS_SIZE);
	const float half_texel = texel_size * 0.5f;

	region.atlas_index = static_cast<uint32_t>(layer);
	region.u_min = static_cast<float>(pixel_x) / ATLAS_SIZE + half_texel;
	region.v_min = static_cast<float>(pixel_y) / ATLAS_SIZE + half_texel;
	region.u_max = static_cast<float>(pixel_x + SPRITE_SIZE) / ATLAS_SIZE - half_texel;
	region.v_max = static_cast<float>(pixel_y + SPRITE_SIZE) / ATLAS_SIZE - half_texel;

	return region;
}

void TextureAtlas::freeSlot(const AtlasRegion& region) {
	free_slots_.push_back(region);
}

void TextureAtlas::bind(uint32_t slot) const {
	glBindTextureUnit(slot, texture_id_->GetID());
}

void TextureAtlas::unbind(uint32_t slot) const {
	glBindTextureUnit(slot, 0);
}

void TextureAtlas::release() {
	if (texture_id_) {
		spdlog::info("TextureAtlas releasing resources [ID={}]", texture_id_->GetID());
	}
	texture_id_.reset();
	layer_count_ = 0;
	allocated_layers_ = 0;
	total_sprite_count_ = 0;
	current_layer_ = 0;
	next_x_ = 0;
	next_y_ = 0;
	free_slots_.clear();
}
