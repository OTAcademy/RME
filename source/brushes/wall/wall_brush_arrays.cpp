//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/wall/wall_brush.h"
#include <array>

namespace {
	// Bitmapping: N=1, W=2, E=4, S=8 (consistent with WALLTILE_ enums)
	enum {
		N = WALLTILE_NORTH,
		W = WALLTILE_WEST,
		E = WALLTILE_EAST,
		S = WALLTILE_SOUTH
	};

	constexpr auto full_border_table = []() constexpr {
		std::array<uint32_t, 16> table {};
		table[0] = WALL_POLE;
		table[N] = WALL_SOUTH_END;
		table[W] = WALL_EAST_END;
		table[N | W] = WALL_NORTHWEST_DIAGONAL;
		table[E] = WALL_WEST_END;
		table[N | E] = WALL_NORTHEAST_DIAGONAL;
		table[W | E] = WALL_HORIZONTAL;
		table[N | W | E] = WALL_SOUTH_T;
		table[S] = WALL_NORTH_END;
		table[N | S] = WALL_VERTICAL;
		table[W | S] = WALL_SOUTHWEST_DIAGONAL;
		table[N | W | S] = WALL_EAST_T;
		table[E | S] = WALL_SOUTHEAST_DIAGONAL;
		table[N | E | S] = WALL_WEST_T;
		table[W | E | S] = WALL_NORTH_T;
		table[N | W | E | S] = WALL_INTERSECTION;
		return table;
	}();

	constexpr auto half_border_table = []() constexpr {
		std::array<uint32_t, 16> table {};
		for (int i = 0; i < 16; ++i) {
			// Half-border ONLY respects N and W bits.
			// This matches the original i % 4 behavior exactly.
			int bits = i & (N | W);
			if (bits == (N | W)) {
				table[i] = WALL_NORTHWEST_DIAGONAL;
			} else if (bits == N) {
				table[i] = WALL_VERTICAL;
			} else if (bits == W) {
				table[i] = WALL_HORIZONTAL;
			} else {
				table[i] = WALL_POLE;
			}
		}
		return table;
	}();

	static_assert(full_border_table[N | S | E | W] == WALL_INTERSECTION);
	static_assert(half_border_table[0] == WALL_POLE);
	static_assert(half_border_table[N] == WALL_VERTICAL);
}

void WallBrush::init() {
	for (int i = 0; i < 16; ++i) {
		WallBrush::full_border_types[i] = full_border_table[i];
		WallBrush::half_border_types[i] = half_border_table[i];
	}
}
