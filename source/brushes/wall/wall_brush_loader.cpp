//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/wall/wall_brush_loader.h"

#include "app/main.h" // Global brushes
#include "brushes/wall/wall_brush.h"
#include "brushes/wall/wall_brush_items.h"
#include "game/items.h"
#include "util/common.h"

#include "wx/arrstr.h"
// pugixml is included via app/main.h
#include <unordered_map>

namespace {

	const std::unordered_map<std::string, BorderType> alignment_map = {
		{ "vertical", WALL_VERTICAL },
		{ "horizontal", WALL_HORIZONTAL },
		{ "corner", WALL_NORTHWEST_DIAGONAL },
		{ "pole", WALL_POLE },
		{ "south end", WALL_SOUTH_END },
		{ "east end", WALL_EAST_END },
		{ "north end", WALL_NORTH_END },
		{ "west end", WALL_WEST_END },
		{ "south T", WALL_SOUTH_T },
		{ "east T", WALL_EAST_T },
		{ "west T", WALL_WEST_T },
		{ "north T", WALL_NORTH_T },
		{ "northwest diagonal", WALL_NORTHWEST_DIAGONAL },
		{ "northeast diagonal", WALL_NORTHEAST_DIAGONAL },
		{ "southwest diagonal", WALL_SOUTHWEST_DIAGONAL },
		{ "southeast diagonal", WALL_SOUTHEAST_DIAGONAL },
		{ "intersection", WALL_INTERSECTION },
		{ "untouchable", WALL_UNTOUCHABLE }
	};

	const std::unordered_map<std::string, ::DoorType> door_type_map = {
		{ "normal", WALL_DOOR_NORMAL },
		{ "normal_alt", WALL_DOOR_NORMAL_ALT },
		{ "locked", WALL_DOOR_LOCKED },
		{ "quest", WALL_DOOR_QUEST },
		{ "magic", WALL_DOOR_MAGIC },
		{ "archway", WALL_ARCHWAY },
		{ "window", WALL_WINDOW },
		{ "hatch_window", WALL_HATCH_WINDOW },
		{ "hatch window", WALL_HATCH_WINDOW }
	};

	// Helper to set door types for "any" shortcuts
	void addAllDoors(WallBrushItems& items, int alignment, uint16_t id, bool locked) {
		items.addDoorItem(alignment, id, WALL_DOOR_NORMAL, locked);
		items.addDoorItem(alignment, id, WALL_DOOR_NORMAL_ALT, locked);
		items.addDoorItem(alignment, id, WALL_DOOR_LOCKED, locked);
		items.addDoorItem(alignment, id, WALL_DOOR_QUEST, locked);
		items.addDoorItem(alignment, id, WALL_DOOR_MAGIC, locked);
		items.addDoorItem(alignment, id, WALL_ARCHWAY, locked);
	}

	void addAllWindows(WallBrushItems& items, int alignment, uint16_t id, bool locked) {
		items.addDoorItem(alignment, id, WALL_WINDOW, locked);
		items.addDoorItem(alignment, id, WALL_HATCH_WINDOW, locked);
	}

	// Helper for C++20 case-insensitive comparison (zero allocation)
	const auto iequal = [](char a, char b) {
		return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
	};

} // namespace

bool WallBrushLoader::load(WallBrush* brush, WallBrushItems& items, pugi::xml_node node, wxArrayString& warnings) {
	pugi::xml_attribute attribute;
	if ((attribute = node.attribute("lookid"))) {
		brush->look_id = attribute.as_ushort();
	}

	if ((attribute = node.attribute("server_lookid"))) {
		brush->look_id = g_items[attribute.as_ushort()].clientID;
	}

	for (pugi::xml_node childNode : node.children()) {
		std::string_view childName = childNode.name();

		if (std::ranges::equal(childName, std::string_view("wall"), iequal)) {
			const std::string typeString = childNode.attribute("type").as_string();
			if (typeString.empty()) {
				warnings.push_back("Could not read type tag of wall node\n");
				continue;
			}

			BorderType alignment = WALL_UNTOUCHABLE;
			// Check alignment map
			auto it = alignment_map.find(typeString);
			if (it != alignment_map.end()) {
				alignment = it->second;
			} else {
				warnings.push_back("Unknown wall alignment '" + wxstr(typeString) + "'\n");
				continue;
			}

			for (pugi::xml_node subChildNode : childNode.children()) {
				std::string_view subChildName = subChildNode.name();

				if (std::ranges::equal(subChildName, std::string_view("item"), iequal)) {
					uint16_t id = subChildNode.attribute("id").as_ushort();
					if (id == 0) {
						warnings.push_back("Could not read id tag of item node\n");
						continue;
					}

					ItemType& it = g_items[id];
					if (it.id == 0) {
						warnings.push_back("There is no itemtype with id " + std::to_string(id));
						return false;
					} else if (it.brush && it.brush != brush) {
						warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
						return false;
					}

					it.isWall = true;
					it.brush = brush;
					it.border_alignment = alignment;

					int chance = subChildNode.attribute("chance").as_int();
					if (chance <= 0) {
						warnings.push_back("Invalid chance for wall item " + std::to_string(id));
						continue;
					}
					items.addWallItem(alignment, id, chance);

				} else if (std::ranges::equal(subChildName, std::string_view("door"), iequal)) {
					uint16_t id = subChildNode.attribute("id").as_ushort();
					if (id == 0) {
						warnings.push_back("Could not read id tag of door node\n");
						continue;
					}

					const std::string type = subChildNode.attribute("type").as_string();
					if (type.empty()) {
						warnings.push_back("Could not read type tag of door node\n");
						continue;
					}

					bool isOpen = true;
					if (pugi::xml_attribute openAttribute = subChildNode.attribute("open")) {
						isOpen = openAttribute.as_bool();
					} else {
						if (type != "window" && type != "any window" && type != "hatch window") {
							warnings.push_back("Could not read open tag of door node\n");
							continue;
						}
					}

					bool isLocked = subChildNode.attribute("locked").as_bool();

					ItemType& it = g_items[id];
					if (it.id == 0) {
						warnings.push_back("There is no itemtype with id " + std::to_string(id));
						return false;
					} else if (it.brush && it.brush != brush) {
						warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
						return false;
					}

					it.isWall = true;
					it.brush = brush;
					it.isBrushDoor = true;
					it.wall_hate_me = subChildNode.attribute("hate").as_bool();
					it.isOpen = isOpen;
					it.isLocked = isLocked;
					it.border_alignment = alignment;

					bool all_windows = false;
					bool all_doors = false;
					::DoorType specificType = WALL_UNDEFINED;

					if (type == "any door") {
						all_doors = true;
					} else if (type == "any window") {
						all_windows = true;
					} else if (type == "any") {
						all_windows = true;
						all_doors = true;
					} else {
						auto doorIt = door_type_map.find(type);
						if (doorIt != door_type_map.end()) {
							specificType = doorIt->second;
						} else {
							warnings.push_back("Unknown door type '" + wxstr(type) + "'\n");
							break;
						}
					}

					if (all_doors) {
						addAllDoors(items, alignment, id, isLocked);
					}
					if (all_windows) {
						addAllWindows(items, alignment, id, isLocked);
					}
					if (!all_doors && !all_windows) {
						items.addDoorItem(alignment, id, specificType, isLocked);
					}
				}
			}
		} else if (std::ranges::equal(childName, std::string_view("friend"), iequal)) {
			const std::string name = childNode.attribute("name").as_string();
			if (name.empty()) {
				continue;
			}

			if (name == "all") {
				// friends.push_back(-1);
			} else {
				Brush* friendBrush = g_brushes.getBrush(name);
				if (friendBrush) {
					brush->friends.push_back(friendBrush->getID());
				} else {
					warnings.push_back("Brush '" + wxstr(name) + "' is not defined.");
					continue;
				}

				if (childNode.attribute("redirect").as_bool()) {
					if (!friendBrush->isWall()) {
						warnings.push_back("Wall brush redirect link: '" + wxstr(name) + "' is not a wall brush.");
					} else if (!brush->redirect_to) {
						brush->redirect_to = friendBrush->asWall();
					} else {
						warnings.push_back("Wall brush '" + wxstr(brush->getName()) + "' has more than one redirect link.");
					}
				}
			}
		}
	}
	return true;
}
