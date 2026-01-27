//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "brushes/brush.h"
#include "brushes/carpet/carpet_brush.h"
#include "brushes/creature/creature_brush.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/ground/auto_border.h"
#include "brushes/house/house_brush.h"
#include "brushes/house/house_exit_brush.h"
#include "brushes/raw/raw_brush.h"
#include "brushes/spawn/spawn_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/waypoint/waypoint_brush.h"
#include "brushes/managers/brush_manager.h"

#include "brushes/flag/flag_brush.h"
#include "brushes/door/door_brush.h"
#include "brushes/border/optional_border_brush.h"

#include "app/settings.h"

#include "game/sprites.h"

#include "game/item.h"
#include "game/complexitem.h"
#include "game/creatures.h"
#include "game/creature.h"
#include "map/map.h"

#include "ui/gui.h"

#include <ranges>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <format>
#include <memory>

Brushes g_brushes;

Brushes::Brushes() {
	////
}

Brushes::~Brushes() {
	////
}

void Brushes::clear() {
	for (auto& entry : brushes) {
		entry.second.reset();
	}
	brushes.clear();

	for (auto& entry : borders) {
		entry.second.reset();
	}
	borders.clear();
}

void Brushes::init() {
	addBrush(g_brush_manager.optional_brush = newd OptionalBorderBrush());
	addBrush(g_brush_manager.eraser = newd EraserBrush());
	addBrush(g_brush_manager.spawn_brush = newd SpawnBrush());
	addBrush(g_brush_manager.normal_door_brush = newd DoorBrush(WALL_DOOR_NORMAL));
	addBrush(g_brush_manager.locked_door_brush = newd DoorBrush(WALL_DOOR_LOCKED));
	addBrush(g_brush_manager.magic_door_brush = newd DoorBrush(WALL_DOOR_MAGIC));
	addBrush(g_brush_manager.quest_door_brush = newd DoorBrush(WALL_DOOR_QUEST));
	addBrush(g_brush_manager.hatch_door_brush = newd DoorBrush(WALL_HATCH_WINDOW));
	addBrush(g_brush_manager.archway_door_brush = newd DoorBrush(WALL_ARCHWAY));
	addBrush(g_brush_manager.normal_door_alt_brush = newd DoorBrush(WALL_DOOR_NORMAL_ALT));
	addBrush(g_brush_manager.window_door_brush = newd DoorBrush(WALL_WINDOW));
	addBrush(g_brush_manager.house_brush = newd HouseBrush());
	addBrush(g_brush_manager.house_exit_brush = newd HouseExitBrush());
	addBrush(g_brush_manager.waypoint_brush = newd WaypointBrush());

	addBrush(g_brush_manager.pz_brush = newd FlagBrush(TILESTATE_PROTECTIONZONE));
	addBrush(g_brush_manager.rook_brush = newd FlagBrush(TILESTATE_NOPVP));
	addBrush(g_brush_manager.nolog_brush = newd FlagBrush(TILESTATE_NOLOGOUT));
	addBrush(g_brush_manager.pvp_brush = newd FlagBrush(TILESTATE_PVPZONE));

	GroundBrush::init();
	WallBrush::init();
	TableBrush::init();
	CarpetBrush::init();
}

bool Brushes::unserializeBrush(pugi::xml_node node, wxArrayString& warnings) {
	pugi::xml_attribute attribute;
	if (!(attribute = node.attribute("name"))) {
		warnings.push_back("Brush node without name.");
		return false;
	}

	const std::string& brushName = attribute.as_string();
	if (brushName == "all" || brushName == "none") {
		warnings.push_back(wxString("Using reserved brushname \"") << wxstr(brushName) << "\".");
		return false;
	}

	Brush* brush = getBrush(brushName);
	if (!brush) {
		attribute = node.attribute("type");
		if (!attribute) {
			warnings.push_back("Couldn't read brush type");
			return false;
		}

		const std::string_view brushType = attribute.as_string();

		static const std::unordered_map<std::string_view, std::function<Brush*()>> typeMap = {
			{ "border", [] { return newd GroundBrush(); } },
			{ "ground", [] { return newd GroundBrush(); } },
			{ "wall", [] { return newd WallBrush(); } },
			{ "wall decoration", [] { return newd WallDecorationBrush(); } },
			{ "carpet", [] { return newd CarpetBrush(); } },
			{ "table", [] { return newd TableBrush(); } },
			{ "doodad", [] { return newd DoodadBrush(); } }
		};

		if (auto it = typeMap.find(brushType); it != typeMap.end()) {
			brush = it->second();
		} else {
			warnings.push_back(wxstr(std::format("Unknown brush type {}", brushType)));
			return false;
		}

		ASSERT(brush);
		brush->setName(brushName);
	}

	if (!node.first_child()) {
		brushes.insert(std::make_pair(brush->getName(), brush));
		return true;
	}

	wxArrayString subWarnings;
	brush->load(node, subWarnings);

	if (!subWarnings.empty()) {
		warnings.push_back(wxString("Errors while loading brush \"") << wxstr(brush->getName()) << "\"");
		warnings.insert(warnings.end(), subWarnings.begin(), subWarnings.end());
	}

	if (brush->getName() == "all" || brush->getName() == "none") {
		warnings.push_back(wxstr(std::format("Using reserved brushname '{}'.", brush->getName())));
		delete brush;
		return false;
	}

	Brush* otherBrush = getBrush(brush->getName());
	if (otherBrush) {
		if (otherBrush != brush) {
			warnings.push_back(wxstr(std::format("Duplicate brush name {}. Undefined behaviour may ensue.", brush->getName())));
		} else {
			// Don't insert
			return true;
		}
	}

	brushes.insert(std::make_pair(brush->getName(), brush));
	return true;
}

bool Brushes::unserializeBorder(pugi::xml_node node, wxArrayString& warnings) {
	pugi::xml_attribute attribute = node.attribute("id");
	if (!attribute) {
		warnings.push_back("Couldn't read border id node");
		return false;
	}

	uint32_t id = attribute.as_uint();
	if (borders.contains(id)) {
		warnings.push_back(wxstr(std::format("Border ID {} already exists", id)));
		return false;
	}

	auto border = std::make_unique<AutoBorder>(id);
	border->load(node, warnings);
	borders[id] = std::move(border);
	return true;
}

void Brushes::addBrush(Brush* brush) {
	brushes.emplace(brush->getName(), std::unique_ptr<Brush>(brush));
}

Brush* Brushes::getBrush(std::string_view name) const {
	if (auto it = brushes.find(name); it != brushes.end()) {
		return it->second.get();
	}
	return nullptr;
}

// Brush
uint32_t Brush::id_counter = 0;
Brush::Brush() :
	id(++id_counter), visible(false), usesCollection(false) {
	////
}

Brush::~Brush() {
	////
}

// TerrainBrush
TerrainBrush::TerrainBrush() :
	look_id(0), hate_friends(false) {
	////
}

TerrainBrush::~TerrainBrush() {
	////
}

bool TerrainBrush::friendOf(TerrainBrush* other) {
	const uint32_t borderID = other->getID();
	const bool found = std::ranges::any_of(friends, [borderID](uint32_t friendId) {
		return friendId == borderID || friendId == 0xFFFFFFFF;
	});
	return found ? !hate_friends : hate_friends;
}
