//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/wall/wall_border_calculator.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/wall/wall_brush_items.h"
#include "map/basemap.h"
#include "game/items.h"
#include "app/main.h" // ASSERT, etc

#include <vector>

bool WallBorderCalculator::hasMatchingWallBrushAtTile(BaseMap* map, WallBrush* wall_brush, int32_t x, int32_t y, int32_t z) {
	Tile* t = map->getTile(x, y, z);
	if (!t) {
		return false;
	}

	for (Item* item : t->items) {
		if (item->isWall()) {
			WallBrush* wb = item->getWallBrush();
			if (wb == wall_brush) {
				return !g_items[item->getID()].wall_hate_me;
			} else if (wb && (wall_brush->friendOf(wb) || wb->friendOf(wall_brush))) {
				return !g_items[item->getID()].wall_hate_me;
			}
		}
	}

	return false;
}

void WallBorderCalculator::doWalls(BaseMap* map, Tile* tile) {
	ASSERT(tile);

	// For quicker reference
	const int32_t x = tile->getPosition().x;
	const int32_t y = tile->getPosition().y;
	const int32_t z = tile->getPosition().z;

	// Advance the vector to the beginning of the walls
	ItemVector::iterator it = tile->items.begin();
	for (; it != tile->items.end() && (*it)->isBorder(); ++it)
		;

	ItemVector items_to_add;

	while (it != tile->items.end()) {
		Item* wall = *it;
		if (!wall->isWall()) {
			++it;
			continue;
		}
		WallBrush* wall_brush = wall->getWallBrush();
		// Skip if either the wall has no brush
		if (!wall_brush) {
			++it;
			continue;
		}
		// or if it's a decoration brush.
		if (wall_brush->isWallDecoration()) {
			items_to_add.push_back(wall);
			it = tile->items.erase(it);
			continue;
		}

		bool neighbours[4] = { false, false, false, false };

		const int32_t dx[] = { 0, -1, 1, 0 };
		const int32_t dy[] = { -1, 0, 0, 1 };

		for (int i = 0; i < 4; ++i) {
			const int32_t tx = x + dx[i];
			const int32_t ty = y + dy[i];
			if (Position(tx, ty, z).isValid()) {
				neighbours[i] = hasMatchingWallBrushAtTile(map, wall_brush, tx, ty, z);
			} else {
				neighbours[i] = false;
			}
		}

		uint32_t tiledata = 0;
		for (int i = 0; i < 4; i++) {
			if (neighbours[i]) {
				// Same wall as this one, calculate what border
				tiledata |= 1 << i;
			}
		}

		bool exit = false;
		for (int i = 0; i < 2; ++i) { // Repeat twice
			if (exit) {
				break;
			}
			const uint32_t* border_tables[] = { WallBrush::full_border_types, WallBrush::half_border_types };
			::BorderType bt = ::BorderType(border_tables[i][tiledata]);

			if (wall->getWallAlignment() == WALL_UNTOUCHABLE) {
				items_to_add.push_back(wall);
				it = tile->items.erase(it);
				exit = true;
			} else if (wall->getWallAlignment() == bt) { // Already correct alignment
				items_to_add.push_back(wall);
				it = tile->items.erase(it);
				exit = true;

				// Handle decorations on top
				while (it != tile->items.end()) {
					Item* wall_decoration = *it;
					ASSERT(wall_decoration);
					WallBrush* brush = wall_decoration->getWallBrush();
					if (brush && brush->isWallDecoration()) {
						if (wall_decoration->getWallAlignment() == bt) {
							// Same, no need to change...
							items_to_add.push_back(wall_decoration);
							it = tile->items.erase(it);
							continue;
						}
						// Create new item with correct alignment
						uint16_t id = brush->items.getRandomWallId(bt);

						if (id != 0) {
							Item* new_wall = Item::Create(id);
							if (wall_decoration->isSelected()) {
								new_wall->select();
							}
							items_to_add.push_back(new_wall);
						}
						++it;
					} else {
						break;
					}
				}
			} else {
				// Randomize a new wall of the proper alignment
				uint16_t id = 0;
				WallBrush* try_brush = wall_brush;

				while (try_brush) {
					id = try_brush->items.getRandomWallId(bt);
					if (id != 0) {
						break;
					}

					try_brush = try_brush->redirect_to;
					if (try_brush == wall_brush) {
						break; // cycle
					}
				}

				if (try_brush == nullptr && id == 0) {
					if (i == 1) {
						++it;
					}
					continue;
				} else {
					// If there is such an item, add it to the tile
					Item* new_wall = Item::Create(id);
					if (wall->isSelected()) {
						new_wall->select();
					}
					items_to_add.push_back(new_wall);
					exit = true;
					++it;
				}

				// Increment and check for deco
				while (it != tile->items.end()) {
					Item* wall_decoration = *it;
					WallBrush* brush = wall_decoration->getWallBrush();
					if (brush && brush->isWallDecoration()) {
						uint16_t id = brush->items.getRandomWallId(bt);
						if (id != 0) {
							Item* new_wall = Item::Create(id);
							if (wall_decoration->isSelected()) {
								new_wall->select();
							}
							items_to_add.push_back(new_wall);
						}
						++it;
					} else {
						// Deliberately skip this non-decoration item and continuing loop with next item
						++it;
						break;
					}
				}
			}
		}
	}
	tile->cleanWalls();
	for (Item* item : items_to_add) {
		tile->addWallItem(item);
	}
}
