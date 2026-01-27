//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/table/table_brush.h"
#include <array>

namespace {
	// Bitmapping: N=2, W=8, E=16, S=64 (consistent with TILE_ enums)
	enum {
		N = TILE_NORTH,
		W = TILE_WEST,
		E = TILE_EAST,
		S = TILE_SOUTH
	};

	constexpr auto table_type_table = []() constexpr {
		std::array<uint32_t, 256> table {};
		for (int i = 0; i < 256; ++i) {
			const bool has_n = (i & N), has_s = (i & S), has_e = (i & E), has_w = (i & W);

			if (has_n && has_s && !has_e && !has_w) {
				table[i] = TABLE_VERTICAL;
			} else if (has_e && has_w && !has_n && !has_s) {
				table[i] = TABLE_HORIZONTAL;
			} else if (has_n && !has_s && !has_e && !has_w) {
				table[i] = TABLE_SOUTH_END;
			} else if (has_s && !has_n && !has_e && !has_w) {
				table[i] = TABLE_NORTH_END;
			} else if (has_e && !has_w && !has_n && !has_s) {
				table[i] = TABLE_WEST_END;
			} else if (has_w && !has_e && !has_n && !has_s) {
				table[i] = TABLE_EAST_END;
			} else {
				table[i] = TABLE_ALONE;
			}
		}
		return table;
	}();

	static_assert(table_type_table[N | S] == TABLE_VERTICAL);
	static_assert(table_type_table[E | W] == TABLE_HORIZONTAL);
}

void TableBrush::init() {
	for (int i = 0; i < 256; ++i) {
		TableBrush::table_types[i] = table_type_table[i];
	}
}
