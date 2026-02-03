#ifndef RME_MAP_SPATIAL_HASH_GRID_H
#define RME_MAP_SPATIAL_HASH_GRID_H

#include "app/main.h"
#include <unordered_map>
#include <cstdint>
#include <memory>

class MapNode;
class BaseMap;

class SpatialHashGrid {
public:
	static constexpr int CELL_SHIFT = 6; // 64 tiles
	static constexpr int CELL_SIZE = 1 << CELL_SHIFT;
	static constexpr int NODE_SHIFT = 2; // 4 tiles
	static constexpr int NODES_PER_CELL = CELL_SIZE >> NODE_SHIFT; // 16 nodes (4x4 tiles each)

	struct GridCell {
		std::unique_ptr<MapNode> nodes[NODES_PER_CELL * NODES_PER_CELL];
		GridCell();
		~GridCell();
	};

	SpatialHashGrid(BaseMap& map);
	~SpatialHashGrid();

	// Returns observer pointer (non-owning)
	MapNode* getLeaf(int x, int y);
	// Forces leaf creation. Throws std::bad_alloc on memory failure.
	MapNode* getLeafForce(int x, int y);

	void clear();
	void clearVisible(uint32_t mask);

	template <typename Func>
	void visitLeaves(int min_x, int min_y, int max_x, int max_y, Func&& func) {
		int start_nx = min_x >> NODE_SHIFT;
		int start_ny = min_y >> NODE_SHIFT;
		int end_nx = (max_x - 1) >> NODE_SHIFT;
		int end_ny = (max_y - 1) >> NODE_SHIFT;

		for (int ny = start_ny; ny <= end_ny; ++ny) {
			for (int nx = start_nx; nx <= end_nx; ++nx) {
				if (MapNode* node = getLeaf(nx << NODE_SHIFT, ny << NODE_SHIFT)) {
					func(node, nx << NODE_SHIFT, ny << NODE_SHIFT);
				}
			}
		}
	}

	auto begin() {
		return cells.begin();
	}
	auto end() {
		return cells.end();
	}

protected:
	BaseMap& map;
	std::unordered_map<uint64_t, std::unique_ptr<GridCell>> cells;

	static uint64_t makeKey(int x, int y) {
		// Assumption: cell coordinates fit in 32 bits (covering +/- 2 billion tiles).
		// Tibia maps are typically limited to 65k x 65k.
		return (static_cast<uint64_t>(static_cast<uint32_t>(x >> CELL_SHIFT)) << 32) | (static_cast<uint64_t>(static_cast<uint32_t>(y >> CELL_SHIFT)));
	}

	friend class BaseMap;
	friend class MapIterator;
};

#endif
