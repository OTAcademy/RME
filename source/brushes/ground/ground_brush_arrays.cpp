//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/ground/ground_brush.h"
#include <array>

namespace {
	// Short aliases for bitmasking in the generator
	enum {
		N = TILE_NORTH,
		S = TILE_SOUTH,
		E = TILE_EAST,
		W = TILE_WEST,
		NW = TILE_NORTHWEST,
		NE = TILE_NORTHEAST,
		SW = TILE_SOUTHWEST,
		SE = TILE_SOUTHEAST
	};

	/**
	 * @brief Generates the border lookup table using RME auto-bordering rules.
	 *
	 * Priority:
	 * 1. Diagonals (A pair of perpendicular sides and no opposite sides).
	 * 2. Horizontal/Vertical sides (if not part of a diagonal).
	 * 3. Corners (Only if both adjacent sides are missing).
	 */
	constexpr auto ground_border_table = []() constexpr {
		std::array<uint32_t, 256> table {};
		for (int i = 0; i < 256; ++i) {
			uint32_t result = 0;
			int shift = 0;
			auto add = [&](uint32_t val) {
				result |= (val << (shift * 8));
				shift++;
			};

			const bool has_n = (i & N);
			const bool has_s = (i & S);
			const bool has_e = (i & E);
			const bool has_w = (i & W);

			// 1. Diagonals: Exclude opposites to ensure we only form a diagonal in simple corners.
			// Complex T-junctions or crosses break back into individual H/V borders.
			const bool nw_d = (has_n && has_w && !has_s && !has_e);
			const bool ne_d = (has_n && has_e && !has_s && !has_w);
			const bool sw_d = (has_s && has_w && !has_n && !has_e);
			const bool se_d = (has_s && has_e && !has_n && !has_w);

			bool n_used = false, s_used = false, e_used = false, w_used = false;

			if (nw_d) {
				add(NORTHWEST_DIAGONAL);
				n_used = w_used = true;
			}
			if (ne_d) {
				add(NORTHEAST_DIAGONAL);
				n_used = e_used = true;
			}
			if (sw_d) {
				add(SOUTHWEST_DIAGONAL);
				s_used = w_used = true;
			}
			if (se_d) {
				add(SOUTHEAST_DIAGONAL);
				s_used = e_used = true;
			}

			// 2. Horizontal / Vertical segments
			if (has_n && !n_used) {
				add(NORTH_HORIZONTAL);
			}
			if (has_s && !s_used) {
				add(SOUTH_HORIZONTAL);
			}
			if (has_e && !e_used) {
				add(EAST_HORIZONTAL);
			}
			if (has_w && !w_used) {
				add(WEST_HORIZONTAL);
			}

			// 3. Corners: Add if and only if adjacent sides are absent.
			if ((i & NW) && !has_n && !has_w) {
				add(NORTHWEST_CORNER);
			}
			if ((i & NE) && !has_n && !has_e) {
				add(NORTHEAST_CORNER);
			}
			if ((i & SW) && !has_s && !has_w) {
				add(SOUTHWEST_CORNER);
			}
			if ((i & SE) && !has_s && !has_e) {
				add(SOUTHEAST_CORNER);
			}

			table[i] = result;
		}
		return table;
	}();

	// Verification of common patterns to ensure logic matches original manual mapping
	static_assert(ground_border_table[N] == NORTH_HORIZONTAL);
	static_assert(ground_border_table[N | W] == NORTHWEST_DIAGONAL);
	static_assert(ground_border_table[N | W | NW] == NORTHWEST_DIAGONAL);
	static_assert(ground_border_table[N | E | W] == (NORTH_HORIZONTAL | EAST_HORIZONTAL << 8 | WEST_HORIZONTAL << 16));
	static_assert(ground_border_table[NW | NE] == (NORTHWEST_CORNER | NORTHEAST_CORNER << 8));
	static_assert(ground_border_table[N | E | SW] == (NORTHEAST_DIAGONAL | SOUTHWEST_CORNER << 8));
}

void GroundBrush::init() {
	// Copy generated table into the static member
	for (int i = 0; i < 256; ++i) {
		GroundBrush::border_types[i] = ground_border_table[i];
	}
}
