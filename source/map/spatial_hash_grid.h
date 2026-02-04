#ifndef RME_MAP_SPATIAL_HASH_GRID_H
#define RME_MAP_SPATIAL_HASH_GRID_H

#include "app/main.h"
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <ranges>

class MapNode;
class BaseMap;

class SpatialHashGrid {
public:
	static constexpr int CELL_SHIFT = 6; // 64 tiles
	static constexpr int CELL_SIZE = 1 << CELL_SHIFT;
	static constexpr int NODE_SHIFT = 2; // 4 tiles
	static constexpr int NODES_PER_CELL_SHIFT = CELL_SHIFT - NODE_SHIFT; // 4
	static constexpr int NODES_PER_CELL = 1 << NODES_PER_CELL_SHIFT; // 16 nodes (4x4 tiles each)
	static constexpr int TILES_PER_NODE = 16; // 4x4 tiles per node

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

	std::vector<std::pair<uint64_t, GridCell*>> getSortedCells() const;

	template <typename Func>
	void visitLeaves(int min_x, int min_y, int max_x, int max_y, Func&& func) {
		int start_nx = min_x >> NODE_SHIFT;
		int start_ny = min_y >> NODE_SHIFT;
		int end_nx = (max_x - 1) >> NODE_SHIFT;
		int end_ny = (max_y - 1) >> NODE_SHIFT;

		uint64_t cached_cell_key = 0;
		bool has_cached_cell = false;
		GridCell* cached_cell = nullptr;

		for (int ny : std::views::iota(start_ny, end_ny + 1)) {
			int cy = ny >> NODES_PER_CELL_SHIFT;
			int local_ny = ny & (NODES_PER_CELL - 1);

			for (int nx : std::views::iota(start_nx, end_nx + 1)) {
				int cx = nx >> NODES_PER_CELL_SHIFT;
				int local_nx = nx & (NODES_PER_CELL - 1);

				uint64_t key = makeKeyFromCell(cx, cy);

				GridCell* cell = nullptr;
				if (has_cached_cell && key == cached_cell_key) {
					cell = cached_cell;
				} else {
					auto it = cells.find(key);
					if (it != cells.end()) {
						cell = it->second.get();
						cached_cell = cell;
						cached_cell_key = key;
						has_cached_cell = true;
					}
				}

				if (cell) {
					int idx = local_ny * NODES_PER_CELL + local_nx;
					if (MapNode* node = cell->nodes[idx].get()) {
						func(node, nx << NODE_SHIFT, ny << NODE_SHIFT);
					}
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

	static uint64_t makeKeyFromCell(int cx, int cy) {
		// Assumption: cell coordinates fit in 32 bits (covering +/- 2 billion tiles).
		// Tibia maps are typically limited to 65k x 65k.
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) | static_cast<uint64_t>(static_cast<uint32_t>(cy));
	}

	static uint64_t makeKey(int x, int y) {
		return makeKeyFromCell(x >> CELL_SHIFT, y >> CELL_SHIFT);
	}

	friend class BaseMap;
	friend class MapIterator;
};

#endif
