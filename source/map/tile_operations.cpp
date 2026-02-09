//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "map/tile_operations.h"
#include "map/tile.h"
#include "game/item.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/carpet/carpet_brush.h"
#include <algorithm>

namespace TileOperations {

void borderize(Tile* tile, BaseMap* map) {
	GroundBrush::doBorders(map, tile);
}

void cleanBorders(Tile* tile) {
	auto& items = tile->items;
	auto first_to_remove = std::stable_partition(items.begin(), items.end(), [](Item* item) {
		return !item->isBorder();
	});

	for (auto it = first_to_remove; it != items.end(); ++it) {
		delete *it;
	}
	items.erase(first_to_remove, items.end());
}

void wallize(Tile* tile, BaseMap* map) {
	WallBrush::doWalls(map, tile);
}

void cleanWalls(Tile* tile, bool dontdelete) {
	auto& items = tile->items;
	auto first_to_remove = std::stable_partition(items.begin(), items.end(), [](Item* item) {
		return !item->isWall();
	});

	if (!dontdelete) {
		for (auto it = first_to_remove; it != items.end(); ++it) {
			delete *it;
		}
	}
	items.erase(first_to_remove, items.end());
}

void cleanWalls(Tile* tile, WallBrush* wb) {
	auto& items = tile->items;
	auto first_to_remove = std::stable_partition(items.begin(), items.end(), [wb](Item* item) {
		return !(item->isWall() && wb->hasWall(item));
	});

	for (auto it = first_to_remove; it != items.end(); ++it) {
		delete *it;
	}
	items.erase(first_to_remove, items.end());
}

void tableize(Tile* tile, BaseMap* map) {
	TableBrush::doTables(map, tile);
}

void cleanTables(Tile* tile, bool dontdelete) {
	auto& items = tile->items;
	auto first_to_remove = std::stable_partition(items.begin(), items.end(), [](Item* item) {
		return !item->isTable();
	});

	if (!dontdelete) {
		for (auto it = first_to_remove; it != items.end(); ++it) {
			delete *it;
		}
	}
	items.erase(first_to_remove, items.end());
}

void carpetize(Tile* tile, BaseMap* map) {
	CarpetBrush::doCarpets(map, tile);
}

} // namespace TileOperations
