//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_BRUSH_TYPES_H
#define RME_DOODAD_BRUSH_TYPES_H

#include "map/position.h"
#include <vector>
#include <utility>
#include <cstdint>

class Item;

#include <memory>

using DoodadItemVector = std::vector<std::unique_ptr<Item>>;
using CompositeTileList = std::vector<std::pair<Position, DoodadItemVector>>;

struct DoodadBrushSettings {
	bool on_blocking = false;
	bool on_duplicate = false;
	bool do_new_borders = false;
	bool one_size = false;
	bool draggable = false;
	uint16_t clear_mapflags = 0;
	uint16_t clear_statflags = 0;
	int thickness = 0;
	int thickness_ceiling = 0;
	uint16_t look_id = 0;
};

#endif
