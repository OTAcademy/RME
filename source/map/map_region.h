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

#ifndef RME_MAP_REGION_H
#define RME_MAP_REGION_H

#include "map/position.h"
#include "map/tile.h"
#include "map/spatial_hash_grid.h"
#include <utility>
#include <unordered_map>

class Tile;
class Floor;
class BaseMap;
class MapNode;

class TileLocation {
	TileLocation();

public:
	~TileLocation();

	TileLocation(const TileLocation&) = delete;
	TileLocation& operator=(const TileLocation&) = delete;

protected:
	std::unique_ptr<Tile> tile;
	Position position;
	size_t spawn_count;
	size_t waypoint_count;
	size_t town_count;
	std::unique_ptr<HouseExitList> house_exits; // Any house exits pointing here

public:
	// Access tile
	// Can't set directly since that does not update tile count
	Tile* get() {
		return tile.get();
	}
	const Tile* get() const {
		return tile.get();
	}

	int size() const;
	bool empty() const;

	Position getPosition() const {
		return position;
	}

	int getX() const {
		return position.x;
	}
	int getY() const {
		return position.y;
	}
	int getZ() const {
		return position.z;
	}

	size_t getSpawnCount() const {
		return spawn_count;
	}
	void increaseSpawnCount() {
		spawn_count++;
	}
	void decreaseSpawnCount() {
		spawn_count--;
	}
	size_t getWaypointCount() const {
		return waypoint_count;
	}
	void increaseWaypointCount() {
		waypoint_count++;
	}
	void decreaseWaypointCount() {
		waypoint_count--;
	}
	size_t getTownCount() const {
		return town_count;
	}
	void increaseTownCount() {
		town_count++;
	}
	void decreaseTownCount() {
		town_count--;
	}
	HouseExitList* createHouseExits() {
		if (house_exits) {
			return house_exits.get();
		}
		house_exits = std::make_unique<HouseExitList>();
		return house_exits.get();
	}
	HouseExitList* getHouseExits() {
		return house_exits.get();
	}

	friend class Floor;
	friend class MapNode;
	friend class Waypoints;
};

class Floor {
public:
	Floor(int x, int y, int z);
	TileLocation locs[MAP_LAYERS];
};

class MapNode {
public:
	MapNode(BaseMap& map);
	~MapNode();

	MapNode(const MapNode&) = delete;
	MapNode& operator=(const MapNode&) = delete;

	TileLocation* createTile(int x, int y, int z);
	TileLocation* getTile(int x, int y, int z);
	Tile* setTile(int x, int y, int z, Tile* tile);
	void clearTile(int x, int y, int z);

	Floor* createFloor(int x, int y, int z);
	Floor* getFloor(uint32_t z) {
		return array[z].get();
	}
	bool hasFloor(uint32_t z);

	void setVisible(bool underground, bool value);
	void setVisible(uint32_t client, bool underground, bool value);
	bool isVisible(uint32_t client, bool underground);
	void clearVisible(uint32_t client);

	bool isRequested(bool underground);
	void setRequested(bool underground, bool r);
	bool isVisible(bool underground);

	enum VisibilityFlags : uint32_t {
		VISIBLE_OVERGROUND = 1 << 0,
		VISIBLE_UNDERGROUND = 1 << 1,
		REQUESTED_UNDERGROUND = 1 << 2,
		REQUESTED_OVERGROUND = 1 << 3,
	};

protected:
	BaseMap& map;
	uint32_t visible;
	std::array<std::unique_ptr<Floor>, MAP_LAYERS> array;

	friend class BaseMap;
	friend class MapIterator;
	friend class SpatialHashGrid;
};

#endif
