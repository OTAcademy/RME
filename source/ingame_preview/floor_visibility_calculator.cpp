#include <iostream>
#include "ingame_preview/floor_visibility_calculator.h"
#include "map/basemap.h"
#include "map/tile.h"
#include "game/items.h"
#include <algorithm>

namespace IngamePreview {

	FloorVisibilityCalculator::FloorVisibilityCalculator() = default;

	bool FloorVisibilityCalculator::TileLimitsFloorsView(const Tile* tile) const {
		if (!tile) {
			return false;
		}

		// In RME, Tile has ground and items.
		// ItemType properties are used to determine blocking.

		if (tile->ground) {
			const ItemType& it = g_items[tile->ground->getID()];
			if (it.isGroundTile()) {
				return true;
			}
			if (it.alwaysOnBottom && it.blockMissiles) {
				return true; // Most walls
			}
		}

		for (const auto& item : tile->items) {
			const ItemType& it = g_items[item->getID()];
			if (it.isGroundTile()) {
				return true;
			}
			if (it.alwaysOnBottom && it.blockMissiles) {
				return true;
			}
		}

		return false;
	}

	bool FloorVisibilityCalculator::IsLookPossible(const Tile* tile) const {
		if (!tile) {
			return true;
		}

		if (tile->ground) {
			const ItemType& it = g_items[tile->ground->getID()];
			if (it.blockMissiles) {
				return false;
			}
		}

		for (const auto& item : tile->items) {
			const ItemType& it = g_items[item->getID()];
			if (it.blockMissiles) {
				return false;
			}
		}

		return true;
	}

	int FloorVisibilityCalculator::CalcFirstVisibleFloor(const BaseMap& map, int camera_x, int camera_y, int camera_z) const {
		int first_floor = 0;

		if (camera_z > GROUND_LAYER) {
			first_floor = std::max(camera_z - AWARE_UNDERGROUND_FLOOR_RANGE, static_cast<int>(GROUND_LAYER) + 1);
		}

		// Check 3x3 area around camera for blocking tiles
		for (int ix = -1; ix <= 1 && first_floor < camera_z; ++ix) {
			for (int iy = -1; iy <= 1 && first_floor < camera_z; ++iy) {
				int pos_x = camera_x + ix;
				int pos_y = camera_y + iy;

				bool is_center = (ix == 0 && iy == 0);
				bool is_diagonal = (std::abs(ix) == std::abs(iy)) && !is_center;

				if (!is_center && is_diagonal) {
					const Tile* current_tile = map.getTile(pos_x, pos_y, camera_z);
					if (!IsLookPossible(current_tile)) {
						continue;
					}
				}

				for (int check_z = camera_z - 1; check_z >= first_floor; --check_z) {
					int z_diff = camera_z - check_z;
					int covered_x = pos_x + z_diff;
					int covered_y = pos_y + z_diff;

					// Check tile directly above
					const Tile* upper_tile = map.getTile(pos_x, pos_y, check_z);
					if (upper_tile && TileLimitsFloorsView(upper_tile)) {
						first_floor = check_z + 1;
						break;
					}

					// Check tile geometrically above (perspective shift)
					const Tile* covered_tile = map.getTile(covered_x, covered_y, check_z);
					if (covered_tile && TileLimitsFloorsView(covered_tile)) {
						first_floor = check_z + 1;
						break;
					}
				}
			}
		}

		return std::clamp(first_floor, 0, static_cast<int>(MAP_MAX_LAYER));
	}

	int FloorVisibilityCalculator::CalcLastVisibleFloor(int camera_z) const {
		int z;
		if (camera_z > GROUND_LAYER) {
			z = camera_z + AWARE_UNDERGROUND_FLOOR_RANGE;
		} else {
			z = GROUND_LAYER;
		}
		return std::clamp(z, 0, static_cast<int>(MAP_MAX_LAYER));
	}

} // namespace IngamePreview
