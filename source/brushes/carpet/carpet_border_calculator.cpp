//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/carpet/carpet_border_calculator.h"
#include "brushes/carpet/carpet_brush.h"
#include "map/basemap.h"
#include "map/tile.h"
#include "game/item.h"
#include <array>

// Helper lambda as a static function
static bool hasMatchingCarpetBrushAtTile(BaseMap* map, CarpetBrush* carpetBrush, uint32_t x, uint32_t y, uint32_t z) {
	Tile* tile = map->getTile(x, y, z);
	if (!tile) {
		return false;
	}

	for (Item* item : tile->items) {
		if (item->getCarpetBrush() == carpetBrush) {
			return true;
		}
	}
	return false;
}

void CarpetBorderCalculator::calculate(BaseMap* map, Tile* tile) {
	ASSERT(tile);
	if (!tile->hasCarpet()) {
		return;
	}

	const Position& position = tile->getPosition();
	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;

	for (Item* item : tile->items) {
		ASSERT(item);

		CarpetBrush* carpetBrush = item->getCarpetBrush();
		if (!carpetBrush) {
			continue;
		}

		std::array<bool, 8> neighbours = { false };

		// Optimize neighbor checking logic
		// Original logic was unrolled. We can keep it unrolled or use loops if cleaner.
		// Original Logic is fine, just modernized slightly using std::array

		if (x == 0) {
			if (y == 0) {
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y, z);
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
			} else {
				neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y - 1, z);
				neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y - 1, z);
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y, z);
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
			}
		} else if (y == 0) {
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y, z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y, z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
		} else {
			neighbours[0] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y - 1, z);
			neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y - 1, z);
			neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y - 1, z);
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y, z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y, z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
		}

		uint32_t tileData = 0;
		for (uint32_t i = 0; i < 8; ++i) {
			if (neighbours[i]) {
				tileData |= static_cast<uint32_t>(1) << i;
			}
		}

		// Accessing friend members of CarpetBrush
		uint16_t id = carpetBrush->getRandomCarpet(static_cast<BorderType>(CarpetBrush::carpet_types[tileData]));
		if (id != 0) {
			item->setID(id);
		}
	}
}
