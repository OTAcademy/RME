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
#include <tuple>

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

		// Cast operands to long long to avoid subtraction overflow before the final multiplication.
		// (static_cast<long long>(end_cx) - start_cx + 1) ensures the expression is evaluated in 64-bit.
		long long num_viewport_cells = (static_cast<long long>(end_cx) - start_cx + 1) * (static_cast<long long>(end_cy) - start_cy + 1);

		// Strategy selection heuristic:
		// If the number of cells in the viewport is greater than the total number of allocated cells,
		// it's more efficient to iterate over allocated cells and check if they are within the viewport.
		// Otherwise, we iterate over the viewport cells and look them up in the hash map.
		// Mixing signed long long and unsigned size_t is safe here as both are non-negative.
		if (num_viewport_cells > static_cast<long long>(cells.size())) {
			visitLeavesByCells(start_nx, start_ny, end_nx, end_ny, start_cx, start_cy, end_cx, end_cy, std::forward<Func>(func));
		} else {
			visitLeavesByViewport(start_nx, start_ny, end_nx, end_ny, start_cx, start_cy, end_cx, end_cy, std::forward<Func>(func));
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

	// Traverses cells by iterating over the viewport coordinates.
	// Efficient for small or dense viewports.
	template <typename Func>
	void visitLeavesByViewport(int start_nx, int start_ny, int end_nx, int end_ny, int start_cx, int start_cy, int end_cx, int end_cy, Func&& func) {
		struct RowCellInfo {
			GridCell* cell;
			int start_nx;
		};
		std::vector<RowCellInfo> row_cells;
		row_cells.reserve(end_cx - start_cx + 1);

		for (int cy = start_cy; cy <= end_cy; ++cy) {
			row_cells.clear();

			for (int cx = start_cx; cx <= end_cx; ++cx) {
				uint64_t key = makeKeyFromCell(cx, cy);
				auto it = cells.find(key);
				if (it != cells.end()) {
					row_cells.push_back({ .cell = it->second.get(), .start_nx = (cx << NODES_PER_CELL_SHIFT) });
				}
			}

			if (row_cells.empty()) {
				continue;
			}

			int row_start_ny = std::max(start_ny, cy << NODES_PER_CELL_SHIFT);
			int row_end_ny = std::min(end_ny, ((cy + 1) << NODES_PER_CELL_SHIFT) - 1);

			for (int ny : std::views::iota(row_start_ny, row_end_ny + 1)) {
				int local_ny = ny & (NODES_PER_CELL - 1);

				for (const auto& [cell, cell_start_nx] : row_cells) {
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

	// Traverses cells by iterating over pre-filtered allocated cells.
	// Efficient for huge or sparse viewports.
	template <typename Func>
	void visitLeavesByCells(int start_nx, int start_ny, int end_nx, int end_ny, int start_cx, int start_cy, int end_cx, int end_cy, Func&& func) {
		struct CellEntry {
			int cx;
			int cy;
			GridCell* cell;
		};
		std::vector<CellEntry> visible_cells;
		visible_cells.reserve(std::min(cells.size(), static_cast<size_t>((static_cast<long long>(end_cx) - start_cx + 1) * (static_cast<long long>(end_cy) - start_cy + 1))));

		for (const auto& [key, cell_ptr] : cells) {
			int cx, cy;
			getCellCoordsFromKey(key, cx, cy);

			if (cx >= start_cx && cx <= end_cx && cy >= start_cy && cy <= end_cy) {
				visible_cells.push_back({ .cx = cx, .cy = cy, .cell = cell_ptr.get() });
			}
		}

		if (visible_cells.empty()) {
			return;
		}

		if (visible_cells.size() > 1) {
			std::sort(visible_cells.begin(), visible_cells.end(), [](const CellEntry& a, const CellEntry& b) {
				return std::tie(a.cy, a.cx) < std::tie(b.cy, b.cx);
			});
		}

		size_t first_in_row_idx = 0;
		size_t num_visible = visible_cells.size();

		for (int ny : std::views::iota(start_ny, end_ny + 1)) {
			int current_cy = ny >> NODES_PER_CELL_SHIFT;
			int local_ny = ny & (NODES_PER_CELL - 1);

			// Advance to current row. This correctly handles gaps in Y coordinates.
			while (first_in_row_idx < num_visible && visible_cells[first_in_row_idx].cy < current_cy) {
				first_in_row_idx++;
			}

			// Iterate cells in current row
			for (size_t i = first_in_row_idx; i < num_visible && visible_cells[i].cy == current_cy; ++i) {
				const auto& entry = visible_cells[i];
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
			}
		}
	}

	static uint64_t makeKeyFromCell(int cx, int cy) {
		static_assert(sizeof(int) == 4, "Key packing assumes exactly 32-bit integers");
		return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) | static_cast<uint32_t>(cy);
	}

	static void getCellCoordsFromKey(uint64_t key, int& cx, int& cy) {
		cx = static_cast<int32_t>(key >> 32);
		cy = static_cast<int32_t>(key);
	}

	static uint64_t makeKey(int x, int y) {
		return makeKeyFromCell(x >> CELL_SHIFT, y >> CELL_SHIFT);
	}

	friend class BaseMap;
	friend class MapIterator;
};

#endif
