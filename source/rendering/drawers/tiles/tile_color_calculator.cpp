#include "app/main.h"
#include "rendering/drawers/tiles/tile_color_calculator.h"
#include "map/tile.h"
#include "game/item.h"
#include "rendering/core/drawing_options.h"
#include "app/definitions.h"
#include <unordered_map>

void TileColorCalculator::Calculate(const Tile* tile, const DrawingOptions& options, uint32_t current_house_id, int spawn_count, uint8_t& r, uint8_t& g, uint8_t& b) {
	bool showspecial = options.show_only_colors || options.show_special_tiles;

	if (options.show_blocking && tile->isBlocking() && tile->size() > 0) {
		g = g / 3 * 2;
		b = b / 3 * 2;
	}

	int item_count = tile->items.size();
	if (options.highlight_items && item_count > 0 && !tile->items.back()->isBorder()) {
		static const float factor[5] = { 0.75f, 0.6f, 0.48f, 0.40f, 0.33f };
		int idx = (item_count < 5 ? item_count : 5) - 1;
		g = int(g * factor[idx]);
		r = int(r * factor[idx]);
	}

	if (options.show_spawns && spawn_count > 0) {
		float f = 1.0f;
		for (int i = 0; i < spawn_count; ++i) {
			f *= 0.7f;
		}
		g = uint8_t(g * f);
		b = uint8_t(b * f);
	}

	if (options.show_houses && tile->isHouseTile()) {
		uint32_t house_id = tile->getHouseID();

		// Get unique house color
		uint8_t hr = 255, hg = 255, hb = 255;
		GetHouseColor(house_id, hr, hg, hb);

		// Apply the house unique color tint to the tile
		r = (uint8_t)((int)r * hr / 255);
		g = (uint8_t)((int)g * hg / 255);
		b = (uint8_t)((int)b * hb / 255);

		if ((int)house_id == current_house_id) {
			// Pulse Effect on top of the unique color
			// We want to make it pulse brighter/intense
			// options.highlight_pulse [0.0, 1.0]

			// Simple intensity boost
			// When pulse is high, we brighten the color towards white
			if (options.highlight_pulse > 0.0f) {
				float boost = options.highlight_pulse * 0.6f; // Max 60% boost towards white

				r = (uint8_t)std::min(255, (int)(r + (255 - r) * boost));
				g = (uint8_t)std::min(255, (int)(g + (255 - g) * boost));
				b = (uint8_t)std::min(255, (int)(b + (255 - b) * boost));
			}
		}
	} else if (showspecial && tile->isPZ()) {
		r /= 2;
		b /= 2;
	}

	if (showspecial && tile->getMapFlags() & TILESTATE_PVPZONE) {
		g = r / 4;
		b = b / 3 * 2;
	}

	if (showspecial && tile->getMapFlags() & TILESTATE_NOLOGOUT) {
		b /= 2;
	}

	if (showspecial && tile->getMapFlags() & TILESTATE_NOPVP) {
		g /= 2;
	}
}

void TileColorCalculator::GetHouseColor(uint32_t house_id, uint8_t& r, uint8_t& g, uint8_t& b) {
	// Use a simple cache to avoid recomputing colors for the same house
	struct Color {
		uint8_t r, g, b;
	};
	static thread_local std::unordered_map<uint32_t, Color> color_cache;

	auto it = color_cache.find(house_id);
	if (it != color_cache.end()) {
		r = it->second.r;
		g = it->second.g;
		b = it->second.b;
		return;
	}

	// Use a simple seeded random to get consistent colors
	// Simple hash
	uint32_t hash = house_id;
	hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
	hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
	hash = (hash >> 16) ^ hash;

	// Generate color components
	r = (hash & 0xFF);
	g = ((hash >> 8) & 0xFF);
	b = ((hash >> 16) & 0xFF);

	// Ensure colors aren't too dark (keep at least one channnel reasonably high)
	if (r < 50 && g < 50 && b < 50) {
		r += 100;
		g += 100;
		b += 100;
	}

	color_cache[house_id] = { r, g, b };
}

void TileColorCalculator::GetMinimapColor(const Tile* tile, uint8_t& r, uint8_t& g, uint8_t& b) {
	uint8_t color = tile->getMiniMapColor();
	r = (uint8_t)(int(color / 36) % 6 * 51);
	g = (uint8_t)(int(color / 6) % 6 * 51);
	b = (uint8_t)(color % 6 * 51);
}
