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

		static constexpr std::array<std::pair<int32_t, int32_t>, 8> offsets = { { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } } };

		for (size_t i = 0; i < offsets.size(); ++i) {
			const auto& [dx, dy] = offsets[i];
			if ((x == 0 && dx < 0) || (y == 0 && dy < 0)) {
				continue;
			}
			neighbours[i] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + dx, y + dy, z);
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
