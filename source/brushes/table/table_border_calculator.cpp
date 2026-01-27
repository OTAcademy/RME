//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/table/table_border_calculator.h"
#include "brushes/table/table_brush.h"
#include "brushes/table/table_brush_items.h"
#include "map/basemap.h"
#include "game/items.h"
#include "app/main.h"

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

		bool neighbours[8];
		if (x == 0) {
			if (y == 0) {
				neighbours[0] = false;
				neighbours[1] = false;
				neighbours[2] = false;
				neighbours[3] = false;
				neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x, y + 1, z);
				neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
			} else {
				neighbours[0] = false;
				neighbours[1] = hasMatchingTableBrushAtTile(map, table_brush, x, y - 1, z);
				neighbours[2] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y - 1, z);
				neighbours[3] = false;
				neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x, y + 1, z);
				neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
			}
		} else if (y == 0) {
			neighbours[0] = false;
			neighbours[1] = false;
			neighbours[2] = false;
			neighbours[3] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y, z);
			neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
			neighbours[5] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x, y + 1, z);
			neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
		} else {
			neighbours[0] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y - 1, z);
			neighbours[1] = hasMatchingTableBrushAtTile(map, table_brush, x, y - 1, z);
			neighbours[2] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y - 1, z);
			neighbours[3] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y, z);
			neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
			neighbours[5] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x, y + 1, z);
			neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
		}

		uint32_t tiledata = 0;
		for (int32_t i = 0; i < 8; ++i) {
			if (neighbours[i]) {
				tiledata |= 1 << i;
			}
		}

		BorderType bt = static_cast<BorderType>(TableBrush::table_types[tiledata]);
		const TableNode& tn = table_brush->items.getItems(static_cast<int32_t>(bt));

		if (tn.total_chance == 0) {
			return;
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
