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

#ifndef RME_MAP_ALLOCATOR_H
#define RME_MAP_ALLOCATOR_H

#include "map/tile.h"
#include "map/map_region.h"

class BaseMap;

class MapAllocator {

public:
	MapAllocator() { }
	~MapAllocator() { }

	// shorthands for tiles
	std::unique_ptr<Tile> operator()(TileLocation* location) {
		return allocateTile(location);
	}

	//
	std::unique_ptr<Tile> allocateTile(TileLocation* location) {
		return std::make_unique<Tile>(*location);
	}

	//
	std::unique_ptr<Floor> allocateFloor(int x, int y, int z) {
		return std::make_unique<Floor>(x, y, z);
	}

	//
	std::unique_ptr<MapNode> allocateNode(BaseMap& map) {
		return std::make_unique<MapNode>(map);
	}
};

#endif
