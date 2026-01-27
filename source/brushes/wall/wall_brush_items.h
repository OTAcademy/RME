//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_WALL_BRUSH_ITEMS_H
#define RME_WALL_BRUSH_ITEMS_H

#include "brushes/brush.h"

#include <vector>
#include <array>
#include <cstdint>
#include <optional>

class WallBrushItems {
public:
	struct WallItem {
		int chance;
		uint16_t id;
	};

	struct WallNode {
		int total_chance = 0;
		std::vector<WallItem> items;
	};

	struct DoorItem {
		::DoorType type;
		uint16_t id;
		bool locked;
	};

	WallBrushItems() = default;

	void addWallItem(int alignment, uint16_t id, int chance);
	void addDoorItem(int alignment, uint16_t id, ::DoorType type, bool locked);

	[[nodiscard]] uint16_t getRandomWallId(int alignment) const;
	[[nodiscard]] bool hasWall(uint16_t id, int alignment) const;
	[[nodiscard]] bool hasDoor(uint16_t id, int alignment) const;
	[[nodiscard]] ::DoorType getDoorTypeFromID(uint16_t id) const;

	// Accessors
	const WallNode& getWallNode(int alignment) const {
		return wall_items.at(alignment);
	}
	const std::vector<DoorItem>& getDoorItems(int alignment) const {
		return door_items.at(alignment);
	}

	// Mutable accessors for advanced manipulation if needed
	WallNode& getWallNode(int alignment) {
		return wall_items.at(alignment);
	}
	std::vector<DoorItem>& getDoorItems(int alignment) {
		return door_items.at(alignment);
	}

private:
	std::array<WallNode, 17> wall_items;
	std::array<std::vector<DoorItem>, 17> door_items;
};

#endif
