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

#include "map/tile.h"
#include "map/basemap.h"
#include "map/spatial_hash_grid.h"

BaseMap::BaseMap() :
	allocator(),
	tilecount(0),
	grid(*this) {
	////
}

BaseMap::~BaseMap() {
	////
}

void BaseMap::clear(bool del) {
	PositionVector pos_vec;
	for (MapIterator map_iter = begin(); map_iter != end(); ++map_iter) {
		Tile* t = map_iter->get();
		pos_vec.push_back(t->getPosition());
	}
	for (PositionVector::iterator pos_iter = pos_vec.begin(); pos_iter != pos_vec.end(); ++pos_iter) {
		setTile(*pos_iter, nullptr, del);
	}
}

void BaseMap::clearVisible(uint32_t mask) {
	grid.clearVisible(mask);
}

Tile* BaseMap::createTile(int x, int y, int z) {
	ASSERT(z < MAP_LAYERS);
	MapNode* leaf = grid.getLeafForce(x, y);
	TileLocation* loc = leaf->createTile(x, y, z);
	if (loc->get()) {
		return loc->get();
	}
	Tile* t = allocator(loc);
	leaf->setTile(x, y, z, t);
	return t;
}

Tile* BaseMap::getOrCreateTile(const Position& pos) {
	if (Tile* t = getTile(pos)) {
		return t;
	}

	Tile* newTile = createTile(pos.x, pos.y, pos.z);
	newTile->setLocation(createTileL(pos));
	return newTile;
}

TileLocation* BaseMap::getTileL(int x, int y, int z) {
	ASSERT(z < MAP_LAYERS);
	MapNode* leaf = grid.getLeaf(x, y);
	if (leaf) {
		Floor* floor = leaf->getFloor(z);
		if (floor) {
			return &floor->locs[(x & 3) * 4 + (y & 3)];
		}
	}

	return nullptr;
}

const TileLocation* BaseMap::getTileL(int x, int y, int z) const {
	// Don't create static const maps!
	BaseMap* self = const_cast<BaseMap*>(this);
	return self->getTileL(x, y, z);
}

TileLocation* BaseMap::getTileL(const Position& pos) {
	return getTileL(pos.x, pos.y, pos.z);
}

const TileLocation* BaseMap::getTileL(const Position& pos) const {
	return getTileL(pos.x, pos.y, pos.z);
}

TileLocation* BaseMap::createTileL(int x, int y, int z) {
	ASSERT(z < MAP_LAYERS);

	MapNode* leaf = grid.getLeafForce(x, y);
	Floor* floor = leaf->createFloor(x, y, z);
	uint32_t offsetX = x & 3;
	uint32_t offsetY = y & 3;

	return &floor->locs[offsetX * 4 + offsetY];
}

TileLocation* BaseMap::createTileL(const Position& pos) {
	return createTileL(pos.x, pos.y, pos.z);
}

void BaseMap::setTile(int x, int y, int z, Tile* newtile, bool remove) {
	ASSERT(!newtile || newtile->getX() == int(x));
	ASSERT(!newtile || newtile->getY() == int(y));
	ASSERT(!newtile || newtile->getZ() == int(z));

	MapNode* leaf = grid.getLeafForce(x, y);
	Tile* old = leaf->setTile(x, y, z, newtile);
	if (remove) {
		delete old;
	}
}

Tile* BaseMap::swapTile(int x, int y, int z, Tile* newtile) {
	ASSERT(z < MAP_LAYERS);
	ASSERT(!newtile || newtile->getX() == int(x));
	ASSERT(!newtile || newtile->getY() == int(y));
	ASSERT(!newtile || newtile->getZ() == int(z));

	MapNode* leaf = grid.getLeafForce(x, y);
	return leaf->setTile(x, y, z, newtile);
}

// Iterators

MapIterator::MapIterator(BaseMap* _map) :
	cell_it(),
	node_i(0),
	floor_i(0),
	tile_i(0),
	current_tile(nullptr),
	map(_map) {
	if (map) {
		cell_it = map->grid.cells.begin();
	}
}

MapIterator::~MapIterator() {
	////
}

// Copy constructor for flat iterator design - copies all stateful members
MapIterator::MapIterator(const MapIterator& other) :
	cell_it(other.cell_it),
	node_i(other.node_i),
	floor_i(other.floor_i),
	tile_i(other.tile_i),
	map(other.map),
	current_tile(other.current_tile) {
}

MapIterator BaseMap::begin() {
	MapIterator it(this);
	it.cell_it = grid.cells.begin();
	it.node_i = 0;
	it.floor_i = 0;
	it.tile_i = 0;
	it.current_tile = nullptr;

	if (it.cell_it != grid.cells.end()) {
		if (!it.findNext()) {
			return end();
		}
	} else {
		return end();
	}
	return it;
}

MapIterator BaseMap::end() {
	MapIterator it(this);
	it.cell_it = grid.cells.end();
	it.current_tile = nullptr;
	return it;
}

TileLocation& MapIterator::operator*() noexcept {
	return *current_tile;
}

bool MapIterator::operator==(const MapIterator& other) const noexcept {
	if (map != other.map) {
		return false;
	}
	if (cell_it == other.cell_it) {
		if (map && cell_it == map->getGrid().cells.end()) {
			return true;
		}
		return node_i == other.node_i && floor_i == other.floor_i && tile_i == other.tile_i;
	}
	return false;
}

bool MapIterator::findNext() {
	while (cell_it != map->grid.cells.end()) {
		SpatialHashGrid::GridCell* cell = cell_it->second.get();
		if (!cell) {
			++cell_it;
			continue;
		}
		while (node_i < SpatialHashGrid::NODES_PER_CELL * SpatialHashGrid::NODES_PER_CELL) {
			MapNode* node = cell->nodes[node_i].get();
			if (node) {
				while (floor_i < MAP_LAYERS) {
					Floor* floor = node->array[floor_i];
					if (floor) {
						while (tile_i < SpatialHashGrid::TILES_PER_NODE) {
							TileLocation& t = floor->locs[tile_i];
							if (t.get()) {
								current_tile = &t;
								return true;
							}
							tile_i++;
						}
					}
					floor_i++;
					tile_i = 0;
				}
			}
			node_i++;
			floor_i = 0;
			tile_i = 0;
		}
		++cell_it;
		node_i = 0;
		floor_i = 0;
		tile_i = 0;
	}
	current_tile = nullptr;
	return false;
}

MapIterator& MapIterator::operator++() noexcept {
	if (current_tile) {
		tile_i++;
		findNext();
	}
	return *this;
}

MapIterator MapIterator::operator++(int) noexcept {
	MapIterator i(*this);
	++*this;
	return i;
}
