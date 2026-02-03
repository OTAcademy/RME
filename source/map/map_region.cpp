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

#include "map/map_region.h"
#include "map/basemap.h"
#include "map/position.h"
#include "map/tile.h"
#include "map/spatial_hash_grid.h"

//**************** Tile Location **********************

TileLocation::TileLocation() :
	tile(nullptr),
	position(0, 0, 0),
	spawn_count(0),
	waypoint_count(0),
	town_count(0),
	house_exits(nullptr) {
	////
}

TileLocation::~TileLocation() {
	delete tile;
	delete house_exits;
}

int TileLocation::size() const {
	if (tile) {
		return tile->size();
	}
	return spawn_count + waypoint_count + (house_exits ? 1 : 0);
}

bool TileLocation::empty() const {
	return size() == 0;
}

//**************** Floor **********************

Floor::Floor(int sx, int sy, int z) {
	sx = sx & ~3;
	sy = sy & ~3;

	for (int i = 0; i < MAP_LAYERS; ++i) {
		locs[i].position.x = sx + (i >> 2);
		locs[i].position.y = sy + (i & 3);
		locs[i].position.z = z;
	}
}

//**************** MapNode **********************

MapNode::MapNode(BaseMap& map) :
	map(map),
	visible(0) {
	for (int i = 0; i < MAP_LAYERS; ++i) {
		array[i] = nullptr;
	}
}

MapNode::~MapNode() {
	for (int i = 0; i < MAP_LAYERS; ++i) {
		delete array[i];
	}
}

Floor* MapNode::createFloor(int x, int y, int z) {
	if (!array[z]) {
		array[z] = newd Floor(x, y, z);
	}
	return array[z];
}

bool MapNode::isVisible(bool underground) {
	return testFlags(visible, underground ? VISIBLE_UNDERGROUND : VISIBLE_OVERGROUND);
}

bool MapNode::isRequested(bool underground) {
	return testFlags(visible, underground ? REQUESTED_UNDERGROUND : REQUESTED_OVERGROUND);
}

void MapNode::clearVisible(uint32_t u) {
	visible &= u;
}

bool MapNode::isVisible(uint32_t client, bool underground) {
	if (underground) {
		return testFlags(visible >> MAP_LAYERS, 1u << client);
	} else {
		return testFlags(visible, 1u << client);
	}
}

void MapNode::setVisible(bool underground, bool value) {
	if (underground) {
		if (value) {
			visible |= VISIBLE_UNDERGROUND;
		} else {
			visible &= ~VISIBLE_UNDERGROUND;
		}
	} else { // overground
		if (value) {
			visible |= VISIBLE_OVERGROUND;
		} else {
			visible &= ~VISIBLE_OVERGROUND;
		}
	}
}

void MapNode::setRequested(bool underground, bool r) {
	uint32_t mask = (underground ? REQUESTED_UNDERGROUND : REQUESTED_OVERGROUND);
	if (r) {
		visible |= mask;
	} else {
		visible &= ~mask;
	}
}

void MapNode::setVisible(uint32_t client, bool underground, bool value) {
	if (value) {
		visible |= (1u << client << (underground ? MAP_LAYERS : 0));
	} else {
		visible &= ~(1u << client << (underground ? MAP_LAYERS : 0));
	}
}

bool MapNode::hasFloor(uint32_t z) {
	return array[z] != nullptr;
}

TileLocation* MapNode::getTile(int x, int y, int z) {
	Floor* f = array[z];
	if (!f) {
		return nullptr;
	}
	return &f->locs[(x & 3) * 4 + (y & 3)];
}

TileLocation* MapNode::createTile(int x, int y, int z) {
	Floor* f = createFloor(x, y, z);
	return &f->locs[(x & 3) * 4 + (y & 3)];
}

Tile* MapNode::setTile(int x, int y, int z, Tile* newtile) {
	Floor* f = createFloor(x, y, z);

	int offset_x = x & 3;
	int offset_y = y & 3;

	TileLocation* tmp = &f->locs[offset_x * 4 + offset_y];
	Tile* oldtile = tmp->tile;
	tmp->tile = newtile;

	if (newtile && !oldtile) {
		++map.tilecount;
	} else if (oldtile && !newtile) {
		--map.tilecount;
	}

	return oldtile;
}

void MapNode::clearTile(int x, int y, int z) {
	Floor* f = createFloor(x, y, z);

	int offset_x = x & 3;
	int offset_y = y & 3;

	TileLocation* tmp = &f->locs[offset_x * 4 + offset_y];
	delete tmp->tile;
	tmp->tile = map.allocator(tmp);
}

//**************** SpatialHashGrid **********************

SpatialHashGrid::GridCell::GridCell() {
	// std::unique_ptr default constructor initializes to nullptr
}

SpatialHashGrid::GridCell::~GridCell() {
	// std::unique_ptr handles cleanup automatically
}

SpatialHashGrid::SpatialHashGrid(BaseMap& map) : map(map) {
	//
}

SpatialHashGrid::~SpatialHashGrid() {
	clear();
}

void SpatialHashGrid::clear() {
	cells.clear();
}

MapNode* SpatialHashGrid::getLeaf(int x, int y) {
	uint64_t key = makeKey(x, y);
	auto it = cells.find(key);
	if (it == cells.end()) {
		return nullptr;
	}

	int nx = (x >> NODE_SHIFT) & (NODES_PER_CELL - 1);
	int ny = (y >> NODE_SHIFT) & (NODES_PER_CELL - 1);
	return it->second->nodes[ny * NODES_PER_CELL + nx].get();
}

MapNode* SpatialHashGrid::getLeafForce(int x, int y) {
	uint64_t key = makeKey(x, y);
	auto& cell = cells[key];
	if (!cell) {
		cell = std::make_unique<GridCell>();
	}

	int nx = (x >> NODE_SHIFT) & (NODES_PER_CELL - 1);
	int ny = (y >> NODE_SHIFT) & (NODES_PER_CELL - 1);
	auto& node = cell->nodes[ny * NODES_PER_CELL + nx];
	if (!node) {
		node = std::make_unique<MapNode>(map);
	}
	return node.get();
}

void SpatialHashGrid::clearVisible(uint32_t mask) {
	for (auto& pair : cells) {
		auto& cell = pair.second;
		if (!cell) {
			continue;
		}
		for (int i = 0; i < NODES_PER_CELL * NODES_PER_CELL; ++i) {
			if (cell->nodes[i]) {
				cell->nodes[i]->clearVisible(mask);
			}
		}
	}
}
