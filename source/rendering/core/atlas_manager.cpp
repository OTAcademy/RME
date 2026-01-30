#include "rendering/core/atlas_manager.h"
#include <iostream>
#include <algorithm>
#include <spdlog/spdlog.h>

bool AtlasManager::ensureInitialized() {
	if (atlas_.isValid()) {
		return true;
	}

	// Pre-allocate 8 layers (8 * 16384 = 131K sprites capacity)
	static constexpr int INITIAL_LAYERS = 8;

	if (!atlas_.initialize(INITIAL_LAYERS)) {
		spdlog::error("AtlasManager: Failed to initialize texture array");
		return false;
	}

	spdlog::info("AtlasManager: Texture array initialized ({}x{}, {} initial layers)", TextureAtlas::ATLAS_SIZE, TextureAtlas::ATLAS_SIZE, INITIAL_LAYERS);

	// Ensure white pixel exists (ID 0xFFFFFFFF)
	std::vector<uint8_t> white_data(32 * 32 * 4, 255);
	addSprite(WHITE_PIXEL_ID, white_data.data());

	return true;
}

const AtlasRegion* AtlasManager::addSprite(uint32_t sprite_id, const uint8_t* rgba_data) {
	// Fast check via direct lookup for common sprites
	if (sprite_id < DIRECT_LOOKUP_SIZE && direct_lookup_[sprite_id] != nullptr) {
		return direct_lookup_[sprite_id];
	}

	// Check hash map for already-added sprites
	auto it = sprite_regions_.find(sprite_id);
	if (it != sprite_regions_.end()) {
		return it->second;
	}

	if (!rgba_data) {
		spdlog::warn("AtlasManager::addSprite called with null data for sprite {}", sprite_id);
		return nullptr;
	}

	if (!ensureInitialized()) {
		return nullptr;
	}

	// Add to texture array
	auto region = atlas_.addSprite(rgba_data);
	if (!region.has_value()) {
		spdlog::error("AtlasManager: Failed to add sprite {} to texture array", sprite_id);
		return nullptr;
	}

	// Store in stable deque
	region_storage_.push_back(*region);
	AtlasRegion* ptr = &region_storage_.back();

	// Store pointer in hash map
	sprite_regions_.emplace(sprite_id, ptr);

	// Also store in direct lookup for O(1) access
	if (sprite_id < DIRECT_LOOKUP_SIZE) {
		direct_lookup_[sprite_id] = ptr;
	}

	return ptr;
}

void AtlasManager::removeSprite(uint32_t sprite_id) {
	if (sprite_id < DIRECT_LOOKUP_SIZE) {
		if (direct_lookup_[sprite_id] != nullptr) {
			const AtlasRegion* region = direct_lookup_[sprite_id];
			atlas_.freeSlot(*region);
			direct_lookup_[sprite_id] = nullptr;
			sprite_regions_.erase(sprite_id);
			// We can't easily remove from region_storage_ (deque), but it's okay, pointers remain valid.
			// The slot in atlas is freed for reuse.
		}
	} else {
		auto it = sprite_regions_.find(sprite_id);
		if (it != sprite_regions_.end()) {
			atlas_.freeSlot(*(it->second));
			sprite_regions_.erase(it);
		}
	}
}

const AtlasRegion* AtlasManager::getWhitePixel() const {
	if (sprite_regions_.count(WHITE_PIXEL_ID)) {
		return sprite_regions_.at(WHITE_PIXEL_ID);
	}
	// Should have been initialized in ensureInitialized()
	return nullptr;
}

bool AtlasManager::hasSprite(uint32_t sprite_id) const {
	if (sprite_id < DIRECT_LOOKUP_SIZE) {
		return direct_lookup_[sprite_id] != nullptr;
	}
	return sprite_regions_.contains(sprite_id);
}

void AtlasManager::bind(uint32_t slot) const {
	atlas_.bind(slot);
}

GLuint AtlasManager::getTextureId() const {
	return atlas_.id();
}

void AtlasManager::clear() {
	atlas_.release();
	region_storage_.clear();
	sprite_regions_.clear();
	std::fill(direct_lookup_.begin(), direct_lookup_.end(), nullptr);
	std::cout << "AtlasManager cleared" << std::endl;
}
