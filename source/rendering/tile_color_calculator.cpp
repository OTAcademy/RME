#include "main.h"
#include "rendering/tile_color_calculator.h"
#include "tile.h"
#include "item.h"
#include "rendering/drawing_options.h"
#include "definitions.h"

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
		if ((int)tile->getHouseID() == current_house_id) {
			r /= 2;
		} else {
			r /= 2;
			g /= 2;
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

void TileColorCalculator::GetMinimapColor(const Tile* tile, uint8_t& r, uint8_t& g, uint8_t& b) {
	uint8_t color = tile->getMiniMapColor();
	r = (uint8_t)(int(color / 36) % 6 * 51);
	g = (uint8_t)(int(color / 6) % 6 * 51);
	b = (uint8_t)(color % 6 * 51);
}
