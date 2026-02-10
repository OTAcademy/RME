#ifndef RME_MAP_SPATIAL_HASH_GRID_H
#define RME_MAP_SPATIAL_HASH_GRID_H

#include "app/main.h"
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <ranges>
#include <algorithm>

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
	static constexpr int NODES_IN_CELL = NODES_PER_CELL * NODES_PER_CELL; // 256 nodes per cell

	struct GridCell {
		std::unique_ptr<MapNode> nodes[NODES_IN_CELL];
		GridCell();
		~GridCell();
	};

	struct SortedGridCell {
		uint64_t key;
		GridCell* cell;
	};

	SpatialHashGrid(BaseMap& map);
	~SpatialHashGrid();

	// Returns observer pointer (non-owning)
	MapNode* getLeaf(int x, int y);
	// Forces leaf creation. Throws std::bad_alloc on memory failure.
	MapNode* getLeafForce(int x, int y);

	void clear();
	void clearVisible(uint32_t mask);

	std::vector<SortedGridCell> getSortedCells() const;

	template <typename Func>
	void visitLeaves(int min_x, int min_y, int max_x, int max_y, Func&& func) {
		int start_nx = min_x >> NODE_SHIFT;
		int start_ny = min_y >> NODE_SHIFT;
		int end_nx = (max_x - 1) >> NODE_SHIFT;
		int end_ny = (max_y - 1) >> NODE_SHIFT;

		int start_cx = start_nx >> NODES_PER_CELL_SHIFT;
		int start_cy = start_ny >> NODES_PER_CELL_SHIFT;
		int end_cx = end_nx >> NODES_PER_CELL_SHIFT;
		int end_cy = end_ny >> NODES_PER_CELL_SHIFT;

		long long num_viewport_cells = (long long)(end_cx - start_cx + 1) * (end_cy - start_cy + 1);

		if (num_viewport_cells > (long long)cells.size()) {
			// Strategy B: Sparse Viewport - Iterate allocated cells
			// This is efficient when the viewport is huge but the map is sparse.

			struct CellEntry {
				int cx;
				int cy;
				GridCell* cell;
			};
			std::vector<CellEntry> visible_cells;
			visible_cells.reserve(std::min((size_t)cells.size(), (size_t)num_viewport_cells));

			for (const auto& entry : cells) {
				uint64_t key = entry.first;
				int cx = static_cast<int>(key >> 32);
				int cy = static_cast<int>(key & 0xFFFFFFFF);

				if (cx >= start_cx && cx <= end_cx && cy >= start_cy && cy <= end_cy) {
					visible_cells.push_back({cx, cy, entry.second.get()});
				}
			}

			// Sort by CY then CX to preserve Painter's Algorithm order (Row-Major)
			std::sort(visible_cells.begin(), visible_cells.end(), [](const CellEntry& a, const CellEntry& b) {
				if (a.cy != b.cy) return a.cy < b.cy;
				return a.cx < b.cx;
			});

			size_t cell_idx = 0;
			size_t num_visible = visible_cells.size();

			for (int ny : std::views::iota(start_ny, end_ny + 1)) {
				int current_cy = ny >> NODES_PER_CELL_SHIFT;
				int local_ny = ny & (NODES_PER_CELL - 1);

				// Advance to current row
				while (cell_idx < num_visible && visible_cells[cell_idx].cy < current_cy) {
					cell_idx++;
				}

				// Iterate cells in current row
				size_t current_idx = cell_idx;
				while (current_idx < num_visible && visible_cells[current_idx].cy == current_cy) {
					const auto& entry = visible_cells[current_idx];
					GridCell* cell = entry.cell;
					int cx = entry.cx;

					int cell_start_nx = cx << NODES_PER_CELL_SHIFT;
					int local_start_nx = std::max(start_nx, cell_start_nx) - cell_start_nx;
					int local_end_nx = std::min(end_nx, cell_start_nx + NODES_PER_CELL - 1) - cell_start_nx;

					for (int lnx = local_start_nx; lnx <= local_end_nx; ++lnx) {
						int idx = local_ny * NODES_PER_CELL + lnx;
						if (MapNode* node = cell->nodes[idx].get()) {
							func(node, (cell_start_nx + lnx) << NODE_SHIFT, ny << NODE_SHIFT);
						}
					}
					current_idx++;
				}
			}
		} else {
			// Strategy A: Dense Viewport - Iterate viewport cells
			// This is efficient when the viewport is small or dense.

			uint64_t cached_cell_key = 0;
			bool has_cached_cell = false;
			GridCell* cached_cell = nullptr;

			for (int ny : std::views::iota(start_ny, end_ny + 1)) {
				int cy = ny >> NODES_PER_CELL_SHIFT;
				int local_ny = ny & (NODES_PER_CELL - 1);

				for (int cx = start_cx; cx <= end_cx; ++cx) {
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
						} else {
							has_cached_cell = false; // Invalidate cache on miss
						}
					}

					if (cell) {
						int cell_start_nx = cx << NODES_PER_CELL_SHIFT;
						int local_start_nx = std::max(start_nx, cell_start_nx) - cell_start_nx;
						int local_end_nx = std::min(end_nx, cell_start_nx + NODES_PER_CELL - 1) - cell_start_nx;

						for (int lnx = local_start_nx; lnx <= local_end_nx; ++lnx) {
							int idx = local_ny * NODES_PER_CELL + lnx;
							if (MapNode* node = cell->nodes[idx].get()) {
								func(node, (cell_start_nx + lnx) << NODE_SHIFT, ny << NODE_SHIFT);
							}
						}
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
