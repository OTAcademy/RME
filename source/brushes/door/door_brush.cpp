//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/door/door_brush.h"

#include "app/settings.h"
#include "brushes/wall/wall_brush.h"
#include "game/item.h"
#include "game/house.h"
#include "game/sprites.h"
#include "map/map.h"
#include "map/tile.h"
#include "ui/gui.h"

#include <ranges>
#include <array>
#include <string_view>
#include <algorithm>

DoorBrush::DoorBrush(DoorType doortype) :
	doortype(doortype) {
	//
}

std::string DoorBrush::getName() const {
	static constexpr std::array<std::pair<DoorType, std::string_view>, 8> doorNames = { { { WALL_DOOR_NORMAL, "Normal door brush" },
																						  { WALL_DOOR_LOCKED, "Locked door brush" },
																						  { WALL_DOOR_MAGIC, "Magic door brush" },
																						  { WALL_DOOR_QUEST, "Quest door brush" },
																						  { WALL_DOOR_NORMAL_ALT, "Normal door (alt) brush" },
																						  { WALL_ARCHWAY, "Archway brush" },
																						  { WALL_WINDOW, "Window brush" },
																						  { WALL_HATCH_WINDOW, "Hatch window brush" } } };

	auto it = std::ranges::find_if(doorNames, [this](const auto& p) { return p.first == doortype; });
	if (it != doorNames.end()) {
		return std::string(it->second);
	}
	return "Unknown door brush";
}

int DoorBrush::getLookID() const {
	static constexpr std::array<std::pair<DoorType, int>, 8> doorSprites = { { { WALL_DOOR_NORMAL, EDITOR_SPRITE_DOOR_NORMAL },
																			   { WALL_DOOR_LOCKED, EDITOR_SPRITE_DOOR_LOCKED },
																			   { WALL_DOOR_MAGIC, EDITOR_SPRITE_DOOR_MAGIC },
																			   { WALL_DOOR_QUEST, EDITOR_SPRITE_DOOR_QUEST },
																			   { WALL_DOOR_NORMAL_ALT, EDITOR_SPRITE_DOOR_NORMAL_ALT },
																			   { WALL_ARCHWAY, EDITOR_SPRITE_DOOR_ARCHWAY },
																			   { WALL_WINDOW, EDITOR_SPRITE_WINDOW_NORMAL },
																			   { WALL_HATCH_WINDOW, EDITOR_SPRITE_WINDOW_HATCH } } };

	auto it = std::ranges::find_if(doorSprites, [this](const auto& p) { return p.first == doortype; });
	if (it != doorSprites.end()) {
		return it->second;
	}
	return EDITOR_SPRITE_DOOR_NORMAL;
}

void DoorBrush::switchDoor(Item* item) {
	ASSERT(item);
	ASSERT(item->isBrushDoor());

	WallBrush* wb = item->getWallBrush();
	if (!wb) {
		return;
	}

	bool new_open = !item->isOpen();
	BorderType wall_alignment = item->getWallAlignment();

	DoorType type = wb->getDoorTypeFromID(item->getID());
	if (type == WALL_UNDEFINED) {
		return;
	}

	uint16_t oppositeVariant = 0;
	bool prefLocked = g_gui.HasDoorLocked();

	const auto& doorItems = wb->items.getDoorItems(wall_alignment);
	for (const auto& dt : doorItems) {
		if (dt.type == type) {
			ASSERT(dt.id);
			ItemType& it = g_items[dt.id];
			ASSERT(it.id != 0);

			if (it.isOpen == new_open) {
				if (!new_open || dt.locked == prefLocked) {
					item->setID(dt.id);
					return;
				}
				oppositeVariant = dt.id;
			}
		}
	}

	if (oppositeVariant != 0) {
		item->setID(oppositeVariant);
	}
}

bool DoorBrush::canDraw(BaseMap* map, const Position& position) const {
	Tile* tile = map->getTile(position);
	if (!tile) {
		return false;
	}

	Item* item = tile->getWall();
	if (!item) {
		return false;
	}

	WallBrush* wb = item->getWallBrush();
	if (!wb) {
		return false;
	}

	BorderType wall_alignment = item->getWallAlignment();
	uint16_t discarded_id = 0;
	bool close_match = false;
	bool open = item->isBrushDoor() && item->isOpen();
	bool prefLocked = g_gui.HasDoorLocked();

	WallBrush* test_brush = wb;
	do {
		const auto& doorItems = test_brush->items.getDoorItems(wall_alignment);
		for (const auto& dt : doorItems) {
			if (dt.type == doortype) {
				ASSERT(dt.id);
				ItemType& it = g_items[dt.id];
				ASSERT(it.id != 0);

				if (it.isOpen == open) {
					if (open || dt.locked == prefLocked) {
						return true;
					}
					discarded_id = dt.id;
					close_match = true;
				} else if (!close_match) {
					discarded_id = dt.id;
					close_match = true;
				}
				if (!close_match && discarded_id == 0) {
					discarded_id = dt.id;
				}
			}
		}
		test_brush = test_brush->redirect_to;
	} while (test_brush != wb && test_brush != nullptr);

	return discarded_id != 0;
}

void DoorBrush::undraw(BaseMap* map, Tile* tile) {
	for (Item* item : tile->items) {
		if (item->isBrushDoor()) {
			item->getWallBrush()->draw(map, tile, nullptr);
			if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				tile->wallize(map);
			}
			return;
		}
	}
}

void DoorBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	for (auto it = tile->items.begin(); it != tile->items.end();) {
		Item* item = *it;
		if (!item->isWall()) {
			++it;
			continue;
		}

		WallBrush* wb = item->getWallBrush();
		if (!wb) {
			++it;
			continue;
		}

		BorderType wall_alignment = item->getWallAlignment();
		uint16_t discarded_id = 0;
		bool close_match = false;
		bool perfect_match = false;
		bool open = parameter ? *static_cast<bool*>(parameter) : (item->isBrushDoor() && item->isOpen());
		bool prefLocked = g_gui.HasDoorLocked();

		WallBrush* test_brush = wb;
		do {
			const auto& doorItems = test_brush->items.getDoorItems(wall_alignment);
			for (const auto& dt : doorItems) {
				if (dt.type == doortype) {
					ASSERT(dt.id);
					ItemType& i_type = g_items[dt.id];
					ASSERT(i_type.id != 0);

					if (i_type.isOpen == open) {
						if (open || dt.locked == prefLocked) {
							item = transformItem(item, dt.id, tile);
							perfect_match = true;
							break;
						}
						discarded_id = dt.id;
						close_match = true;
					} else if (!close_match) {
						discarded_id = dt.id;
						close_match = true;
					}
					if (!close_match && discarded_id == 0) {
						discarded_id = dt.id;
					}
				}
			}
			if (perfect_match) {
				break;
			}
			test_brush = test_brush->redirect_to;
		} while (test_brush != wb && test_brush != nullptr);

		if (!perfect_match && discarded_id) {
			item = transformItem(item, discarded_id, tile);
		}

		if (g_settings.getInteger(Config::AUTO_ASSIGN_DOORID) && tile->isHouseTile()) {
			if (auto* mmap = dynamic_cast<Map*>(map)) {
				if (auto* door = dynamic_cast<Door*>(item)) {
					if (House* house = mmap->houses.getHouse(tile->getHouseID())) {
						door->setDoorID(house->getEmptyDoorID());
					}
				}
			}
		}

		// Decorations
		while (true) {
			auto found_it = std::ranges::find(tile->items, item);
			if (found_it == tile->items.end()) {
				return;
			}

			it = ++found_it;
			if (it == tile->items.end()) {
				return;
			}

			item = *it;
			if (item->isWall()) {
				if (WallBrush* brush = item->getWallBrush(); brush && brush->isWallDecoration()) {
					perfect_match = false;
					close_match = false;
					discarded_id = 0;

					const auto& decDoorItems = brush->items.getDoorItems(wall_alignment);
					for (const auto& dt : decDoorItems) {
						if (dt.type == doortype) {
							ASSERT(dt.id);
							ItemType& i_type = g_items[dt.id];
							ASSERT(i_type.id != 0);

							if (i_type.isOpen == open) {
								if (open || dt.locked == prefLocked) {
									item = transformItem(item, dt.id, tile);
									perfect_match = true;
									break;
								}
								discarded_id = dt.id;
								close_match = true;
							} else if (!close_match) {
								discarded_id = dt.id;
								close_match = true;
							}
							if (!close_match && discarded_id == 0) {
								discarded_id = dt.id;
							}
						}
					}
					if (!perfect_match && discarded_id) {
						item = transformItem(item, discarded_id, tile);
					}
					continue;
				}
			}
			break;
		}
		return;
	}
}
