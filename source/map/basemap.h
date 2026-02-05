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

#ifndef RME_BASE_MAP_H_
#define RME_BASE_MAP_H_

#include "app/main.h"
#include "map/position.h"
#include "io/filehandle.h"
#include "map/map_allocator.h"
#include "map/tile.h"
#include "map/spatial_hash_grid.h"
#include <unordered_map>
#include <memory>
#include <iterator>

// Class declarations
class SpatialHashGrid;
class BaseMap;
class MapIterator;
class Floor;
class MapNode;
class TileLocation;

class MapIterator {
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = TileLocation;
	using difference_type = std::ptrdiff_t;
	using pointer = TileLocation*;
	using reference = TileLocation&;

	MapIterator(BaseMap* _map = nullptr);
	~MapIterator();
	MapIterator(const MapIterator& other);

	TileLocation& operator*() noexcept;
	TileLocation* operator->() noexcept {
		return &(*(*this));
	}
	const TileLocation* operator->() const noexcept {
		return &(*(*const_cast<MapIterator*>(this)));
	}
	MapIterator& operator++() noexcept;
	MapIterator operator++(int) noexcept;
	bool operator==(const MapIterator& other) const noexcept;
	bool operator!=(const MapIterator& other) const noexcept {
		return !(other == *this);
	}

private:
	bool findNext();

	using CellIterator = std::unordered_map<uint64_t, std::unique_ptr<SpatialHashGrid::GridCell>>::iterator;
	CellIterator cell_it;
	int node_i, floor_i, tile_i;

	TileLocation* current_tile;
	BaseMap* map;

	friend class BaseMap;
};

class BaseMap {
public:
	BaseMap();
	virtual ~BaseMap();

	// This doesn't destroy the map structure, just clears it, if param is true, delete all tiles too.
	void clear(bool del = true);
	MapIterator begin();
	MapIterator end();
	uint64_t size() const {
		return tilecount;
	}

	// these functions take a position and returns a tile on the map
	Tile* createTile(int x, int y, int z);
	Tile* getTile(int x, int y, int z);
	Tile* getTile(const Position& pos);
	Tile* getOrCreateTile(const Position& pos);
	const Tile* getTile(int x, int y, int z) const;
	const Tile* getTile(const Position& pos) const;
	TileLocation* getTileL(int x, int y, int z);
	TileLocation* getTileL(const Position& pos);
	TileLocation* createTileL(int x, int y, int z);
	TileLocation* createTileL(const Position& pos);
	const TileLocation* getTileL(int x, int y, int z) const;
	const TileLocation* getTileL(const Position& pos) const;

	// Get a Map Node from the map
	MapNode* getLeaf(int x, int y) {
		return grid.getLeaf(x, y);
	}
	MapNode* createLeaf(int x, int y) {
		return grid.getLeafForce(x, y);
	}

	template <typename Func>
	void visitLeaves(int min_x, int min_y, int max_x, int max_y, Func&& func) {
		grid.visitLeaves(min_x, min_y, max_x, max_y, std::forward<Func>(func));
	}

	// Assigns a tile, it might seem pointless to provide position, but it is not, as the passed tile may be nullptr
	void setTile(int _x, int _y, int _z, Tile* newtile, bool remove = false);
	void setTile(const Position& pos, Tile* newtile, bool remove = false) {
		setTile(pos.x, pos.y, pos.z, newtile, remove);
	}
	void setTile(Tile* newtile, bool remove = false) {
		setTile(newtile->getX(), newtile->getY(), newtile->getZ(), newtile, remove);
	}
	// Replaces a tile and returns the old one
	Tile* swapTile(int _x, int _y, int _z, Tile* newtile);
	Tile* swapTile(const Position& pos, Tile* newtile) {
		return swapTile(pos.x, pos.y, pos.z, newtile);
	}

	SpatialHashGrid& getGrid() {
		return grid;
	}
	const SpatialHashGrid& getGrid() const {
		return grid;
	}

	// Clears the visiblity according to the mask passed
	void clearVisible(uint32_t mask);

	uint64_t getTileCount() const {
		return tilecount;
	}

public:
	MapAllocator allocator;

protected:
	uint64_t tilecount;

	SpatialHashGrid grid; // The Spatial Hash Grid

	friend class MapNode;
	friend class MapProcessor;
	friend class EditorPersistence;
	friend class MapIterator;
};

inline Tile* BaseMap::getTile(int x, int y, int z) {
	TileLocation* l = getTileL(x, y, z);
	return l ? l->get() : nullptr;
}

inline Tile* BaseMap::getTile(const Position& pos) {
	TileLocation* l = getTileL(pos);
	return l ? l->get() : nullptr;
}

inline const Tile* BaseMap::getTile(int x, int y, int z) const {
	const TileLocation* l = getTileL(x, y, z);
	return l ? l->get() : nullptr;
}

inline const Tile* BaseMap::getTile(const Position& pos) const {
	const TileLocation* l = getTileL(pos);
	return l ? l->get() : nullptr;
}

#endif
