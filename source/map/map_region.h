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
#include <utility>

class Tile;
class Floor;
class BaseMap;

class TileLocation {
	TileLocation();

public:
	~TileLocation();

	TileLocation(const TileLocation&) = delete;
	TileLocation& operator=(const TileLocation&) = delete;

protected:
	Tile* tile;
	Position position;
	size_t spawn_count;
	size_t waypoint_count;
	size_t town_count;
	HouseExitList* house_exits; // Any house exits pointing here

public:
	// Access tile
	// Can't set directly since that does not update tile count
	Tile* get() {
		return tile;
	}
	const Tile* get() const {
		return tile;
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
			return house_exits;
		}
		return house_exits = newd HouseExitList;
	}
	HouseExitList* getHouseExits() {
		return house_exits;
	}

	friend class Floor;
	friend class QTreeNode;
	friend class Waypoints;
};

class Floor {
public:
	Floor(int x, int y, int z);
	TileLocation locs[MAP_LAYERS];
};

// This is not a QuadTree, but a HexTree (16 child nodes to every node), so the name is abit misleading
class QTreeNode {
public:
	QTreeNode(BaseMap& map);
	virtual ~QTreeNode();

	QTreeNode(const QTreeNode&) = delete;
	QTreeNode& operator=(const QTreeNode&) = delete;

	QTreeNode* getLeaf(int x, int y); // Might return nullptr
	QTreeNode* getLeafForce(int x, int y); // Will never return nullptr, it will create the node if it's not there

	// Coordinates are NOT relative
	TileLocation* createTile(int x, int y, int z);
	TileLocation* getTile(int x, int y, int z);
	Tile* setTile(int x, int y, int z, Tile* tile);
	void clearTile(int x, int y, int z);

	template <typename Func>
	void visitLeaves(int x, int y, int size, int min_x, int min_y, int max_x, int max_y, Func&& func) {
		if (x >= max_x || y >= max_y || x + size <= min_x || y + size <= min_y) {
			return;
		}

		if (isLeaf) {
			func(this, x, y);
			return;
		}

		int child_size = size / 4;
		for (int iy = 0; iy < 4; ++iy) {
			int cy = y + iy * child_size;
			if (cy >= max_y || cy + child_size <= min_y) {
				continue;
			}

			for (int ix = 0; ix < 4; ++ix) {
				int cx = x + ix * child_size;
				if (cx >= max_x || cx + child_size <= min_x) {
					continue;
				}

				int index = (iy << 2) | ix;
				if (child[index]) {
					child[index]->visitLeaves(cx, cy, child_size, min_x, min_y, max_x, max_y, std::forward<Func>(func));
				}
			}
		}
	}

	Floor* createFloor(int x, int y, int z);
	Floor* getFloor(uint32_t z) {
		ASSERT(isLeaf);
		return array[z];
	}
	Floor** getFloors() {
		return array;
	}

	void setVisible(bool overground, bool underground);
	void setVisible(uint32_t client, bool underground, bool value);
	bool isVisible(uint32_t client, bool underground);
	void clearVisible(uint32_t client);

	void setRequested(bool underground, bool r);
	bool isVisible(bool underground);
	bool isRequested(bool underground);

protected:
	BaseMap& map;
	uint32_t visible;

	bool isLeaf;
	union {
		QTreeNode* child[MAP_LAYERS];
		Floor* array[MAP_LAYERS];
		/*
		#if 16 != MAP_LAYERS
		#    error "You need to rewrite the QuadTree in order to handle more or less than 16 floors"
		#endif
		*/
	};

	friend class BaseMap;
	friend class MapIterator;
};

#endif
