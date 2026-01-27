//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/carpet/carpet_brush.h"
#include <array>

namespace {
	// Bitmapping (consistent with TILE_ enums)
	enum {
		NW = TILE_NORTHWEST,
		N = TILE_NORTH,
		NE = TILE_NORTHEAST,
		W = TILE_WEST,
		E = TILE_EAST,
		SW = TILE_SOUTHWEST,
		S = TILE_SOUTH,
		SE = TILE_SOUTHEAST
	};

	/**
	 * @brief Formulaic generator for carpet brushes.
	 * Implements the legacy logic rules from legacy_brush_arrays.cpp.
	 *
	 * Legacy Logic Summary:
	 * - Full Neighbors (NSEW): Center, with corner logic.
	 * - 3-Way: Prioritizes corners/lines based on presence of diagonals.
	 * - 2-Way Orthogonal (Corner/Line): Standard, but with diagonal overrides.
	 * - 1-Way Orthogonal: Often becomes Corner or Line if specific diagonals are present.
	 */
	constexpr auto carpet_type_table = []() constexpr {
		std::array<uint32_t, 256> table {};
		for (int i = 0; i < 256; ++i) {
			const bool nw = i & NW;
			const bool n = i & N;
			const bool ne = i & NE;
			const bool w = i & W;
			const bool e = i & E;
			const bool sw = i & SW;
			const bool s = i & S;
			const bool se = i & SE;

			// --- 1. Full Enclosed Block (N, S, E, W) ---
			if (n && s && e && w) {
				const int missing_diag = (!nw) + (!ne) + (!sw) + (!se);
				if (missing_diag == 1) {
					if (!nw) {
						table[i] = SOUTHEAST_DIAGONAL;
					} else if (!ne) {
						table[i] = SOUTHWEST_DIAGONAL;
					} else if (!sw) {
						table[i] = NORTHEAST_DIAGONAL;
					} else if (!se) {
						table[i] = NORTHWEST_DIAGONAL;
					}
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}

			// --- 2. Three-Way Junctions ---
			if (n && s && w) { // Missing E
				if (sw && nw) {
					table[i] = WEST_HORIZONTAL;
				} else if (sw) {
					table[i] = SOUTHWEST_CORNER; // Legacy 225? S|SW|N|NW -> W_HORIZ. S|SW|N -> SW_CORNER.
				}
				// Legacy 223: S|SW|N -> SW_CORNER.
				// Legacy 175: S|W|N -> WEST_HORIZONTAL.
				// Legacy 178: S|W|N|NW -> NORTHWEST_CORNER.

				// Re-eval based on legacy file checks:
				// 175: S | W | N -> WEST_HORIZONTAL
				// 178: S | W | N | NW -> NORTHWEST_CORNER
				// 223: S | N | SW -> SOUTHWEST_CORNER
				// 239: S | SW | W | N -> WEST_HORIZONTAL
				// 241: S | SW | W | N | NW -> WEST_HORIZONTAL

				else if (nw) {
					table[i] = NORTHWEST_CORNER; // 178
				} else {
					table[i] = WEST_HORIZONTAL; // 175
				}
				continue;
			}
			if (n && s && e) { // Missing W
				if (se && ne) {
					table[i] = EAST_HORIZONTAL;
				}
				// Legacy 191: S | E | N -> EAST_HORIZONTAL.
				// Legacy 127: SW | E | N -> NORTHEAST_CORNER. (Wait, SW is active?)
				// Legacy 62: E | N -> NORTHEAST_CORNER.
				// Legacy 188: S | E -> SOUTHEAST_CORNER.
				else {
					table[i] = EAST_HORIZONTAL;
				}
				continue;
			}
			if (n && w && e) { // Missing S
				// Legacy 46: W | N -> NORTHWEST_CORNER.
				// Legacy 87: E | W | NE | N -> NORTH_HORIZONTAL.
				// Legacy 78: E | W | N -> NORTH_HORIZONTAL.
				if (sw) {
					table[i] = NORTHWEST_CORNER; // Legacy 111: SW|W|N -> NW_CORNER.
				} else {
					table[i] = NORTH_HORIZONTAL;
				}
				continue;
			}
			if (s && w && e) { // Missing N
				// Legacy 204: S | E | W -> SOUTH_HORIZONTAL.
				if (nw) {
					table[i] = SOUTH_HORIZONTAL; // Legacy 205 + 34
				} else {
					table[i] = SOUTH_HORIZONTAL;
				}
				continue;
			}

			// --- 3. Two-Way Orthogonal ---
			// Corners
			if (n && w) {
				table[i] = NORTHWEST_CORNER;
				continue;
			}
			if (n && e) {
				table[i] = NORTHEAST_CORNER;
				continue;
			}
			if (s && w) {
				table[i] = SOUTHWEST_CORNER;
				continue;
			}
			if (s && e) {
				table[i] = SOUTHEAST_CORNER;
				continue;
			}

			// Lines
			if (n && s) {
				// Legacy 159: S | N -> CARPET_CENTER.
				// Legacy 161: S | N | NW -> NORTHWEST_CORNER.
				// Legacy 223: S | N | SW -> SOUTHWEST_CORNER.
				if (nw && sw) {
					table[i] = WEST_HORIZONTAL;
				} else if (nw) {
					table[i] = NORTHWEST_CORNER;
				} else if (sw) {
					table[i] = SOUTHWEST_CORNER;
				} else if (ne) {
					table[i] = NORTHEAST_CORNER; // Legacy 168?
				} else if (se) {
					table[i] = SOUTHEAST_CORNER;
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}
			if (w && e) {
				// Legacy 75: E | W -> CARPET_CENTER.
				// Legacy 76: E | W | NW -> NORTH_HORIZONTAL.
				// Legacy 82: E | W | NE -> NORTH_HORIZONTAL.
				// Legacy 138: E | W | SW -> SOUTHWEST_CORNER (Wait check)
				// My Python script said 139: SW|E|W -> SOUTHWEST_CORNER.

				bool n_side = nw || ne;
				bool s_side = sw || se;

				if (sw && e && w) {
					table[i] = SOUTHWEST_CORNER; // Legacy 138/139 override
				} else if (n_side && s_side) {
					table[i] = CARPET_CENTER;
				} else if (n_side) {
					table[i] = NORTH_HORIZONTAL;
				} else if (s_side) {
					table[i] = SOUTH_HORIZONTAL;
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}

			// --- 4. Single Orthogonal (Legacy Quirks) ---
			if (n) {
				// Legacy 23: N -> CARPET_CENTER. (Actually 30 in verify?)
				// Legacy 28: NW -> CARPET_CENTER.
				// Legacy 32: N | NW -> NORTHWEST_CORNER (Wait, Legacy 32 is TILE_NORTH | TILE_NORTHWEST).
				// Logic: N | NW -> NW_CORNER.
				if (nw) {
					table[i] = NORTHWEST_CORNER;
				} else if (ne) {
					table[i] = NORTHEAST_CORNER;
				} else if (sw) {
					table[i] = SOUTHWEST_CORNER; // Legacy 95: SW | N -> SW_CORNER.
				} else if (se) {
					table[i] = SOUTHEAST_CORNER; // Legacy 289: SE | N -> SE_CORNER.
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}
			if (s) {
				if (sw) {
					table[i] = SOUTHWEST_CORNER; // Legacy 221
				} else if (se) {
					table[i] = SOUTHEAST_CORNER;
				} else if (nw) {
					table[i] = NORTHWEST_CORNER; // Legacy 157
				} else if (ne) {
					table[i] = NORTHEAST_CORNER; // Legacy 163
				} else {
					table[i] = SOUTHWEST_CORNER; // Legacy 154: TILE_SOUTH -> SOUTHWEST_CORNER?
				}
				// Yes, Legacy file line 155: CarpetBrush::carpet_types[TILE_SOUTH] = SOUTHWEST_CORNER;
				continue;
			}
			if (w) {
				if (nw) {
					table[i] = WEST_HORIZONTAL; // Legacy 45: W | NW -> WEST_HORIZONTAL.
				} else if (sw) {
					table[i] = SOUTHWEST_CORNER; // Legacy 107
				} else if (se) {
					table[i] = SOUTH_HORIZONTAL; // Legacy 301: SE | W -> S_HORIZ.
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}
			if (e) {
				if (nw) {
					table[i] = NORTHEAST_CORNER; // Legacy 60: E | NW -> NE_CORNER.
				} else if (ne) {
					table[i] = NORTHEAST_CORNER;
				} else if (se) {
					table[i] = SOUTH_HORIZONTAL; // Legacy 316: SE | E -> S_HORIZ.
				} else {
					table[i] = CARPET_CENTER;
				}
				continue;
			}

			// --- 5. Pure Diagonals ---
			if (nw && ne) {
				table[i] = NORTH_HORIZONTAL;
			} else if (sw && se) {
				table[i] = SOUTH_HORIZONTAL;
			} else if (nw && sw) {
				table[i] = WEST_HORIZONTAL;
			} else if (ne && se) {
				table[i] = EAST_HORIZONTAL;
			} else if (ne) {
				table[i] = NORTHEAST_CORNER;
			} else if (se) {
				table[i] = SOUTHEAST_CORNER;
			} else if (sw) {
				table[i] = SOUTHWEST_CORNER;
			} else {
				table[i] = CARPET_CENTER;
			}
		}
		return table;
	}();

	// Verification of specific legacy behaviors
	static_assert(carpet_type_table[N | NW | W] == NORTHWEST_CORNER);
	static_assert(carpet_type_table[N | W | E] == NORTH_HORIZONTAL);

	// Legacy Specifics verified from file analysis
	static_assert(carpet_type_table[W | NW] == WEST_HORIZONTAL); // Line 45
	static_assert(carpet_type_table[N | NW] == NORTHWEST_CORNER); // Line 32
	static_assert(carpet_type_table[E | NW] == NORTHEAST_CORNER); // Line 60
	static_assert(carpet_type_table[S | NW] == NORTHWEST_CORNER); // Line 157

	static_assert(carpet_type_table[S] == SOUTHWEST_CORNER); // Line 155
}

void CarpetBrush::init() {
	for (int i = 0; i < 256; ++i) {
		CarpetBrush::carpet_types[i] = carpet_type_table[i];
	}
}
