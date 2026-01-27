//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/table/table_border_calculator.h"
#include "brushes/table/table_brush.h"
#include "brushes/table/table_brush_items.h"
#include "map/basemap.h"
#include "game/items.h"
#include "app/main.h"
#include <array>

// TableBorderCalculator is a friend of TableBrush so it can access protected members.

bool TableBorderCalculator::hasMatchingTableBrushAtTile(BaseMap* map, TableBrush* table_brush, int32_t x, int32_t y, int32_t z) {
	Tile* t = map->getTile(x, y, z);
	if (!t) {
		return false;
	}

	for (Item* item : t->items) {
		TableBrush* tb = item->getTableBrush();
		if (tb == table_brush) {
			return true;
		}
	}

	return false;
}

void TableBorderCalculator::doTables(BaseMap* map, Tile* tile) {
	ASSERT(tile);
	if (!tile->hasTable()) {
		return;
	}

	const Position& position = tile->getPosition();
	int32_t x = position.x;
	int32_t y = position.y;
	int32_t z = position.z;

	for (Item* item : tile->items) {
		ASSERT(item);

		TableBrush* table_brush = item->getTableBrush();
		if (!table_brush) {
			continue;
		}

		// Neighbors order:
		// 0: top-left (-1, -1)
		// 1: top (0, -1)
		// 2: top-right (1, -1)
		// 3: left (-1, 0)
		// 4: right (1, 0)
		// 5: bottom-left (-1, 1)
		// 6: bottom (0, 1)
		// 7: bottom-right (1, 1)

		static constexpr std::array<std::pair<int32_t, int32_t>, 8> offsets = { { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } } };

		uint32_t tiledata = 0;
		for (size_t i = 0; i < offsets.size(); ++i) {
			const auto& [dx, dy] = offsets[i];
			// Check if neighbor is valid (within bounds/logic of hasMatchingTableBrushAtTile)

			int32_t nx = x + dx;
			int32_t ny = y + dy;

			if (nx < 0 || ny < 0) { // Basic sanity check matching original logic implicitly
				continue;
			}

			if (hasMatchingTableBrushAtTile(map, table_brush, nx, ny, z)) {
				tiledata |= 1 << i;
			}
		}

		BorderType bt = static_cast<BorderType>(TableBrush::table_types[tiledata]);
		const TableNode& tn = table_brush->items.getItems(static_cast<int32_t>(bt));

		if (tn.total_chance == 0) {
			continue;
		}

		int32_t chance = random(1, tn.total_chance);
		uint16_t id = 0;

		for (const auto& tableType : tn.items) {
			if (chance <= tableType.chance) {
				id = tableType.item_id;
				break;
			}
			chance -= tableType.chance;
		}

		if (id != 0) {
			item->setID(id);
		}
	}
}
