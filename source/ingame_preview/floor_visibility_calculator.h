#ifndef RME_INGAME_PREVIEW_FLOOR_VISIBILITY_CALCULATOR_H_
#define RME_INGAME_PREVIEW_FLOOR_VISIBILITY_CALCULATOR_H_

#include "app/main.h"
#include "map/position.h"

class BaseMap;
class Tile;
class Item;

namespace IngamePreview {

	/**
	 * Calculates floor visibility for client-accurate rendering in RME.
	 *
	 * Implements OTClient's floor visibility algorithm:
	 * - On surface (Z <= 7): Sees floors 0-7, upper floors hidden by roofs
	 * - Underground (Z > 7): Sees Z ± 2 floors
	 */
	class FloorVisibilityCalculator {
	public:
		FloorVisibilityCalculator();
		~FloorVisibilityCalculator() = default;

		/**
		 * Calculate first (topmost) visible floor from camera position.
		 */
		int CalcFirstVisibleFloor(const BaseMap& map, int camera_x, int camera_y, int camera_z) const;

		/**
		 * Calculate last (deepest) visible floor from camera position.
		 */
		int CalcLastVisibleFloor(int camera_z) const;

		/**
		 * Check if a tile limits the view of floors above it (roofs/solid ground).
		 */
		bool TileLimitsFloorsView(const Tile* tile) const;

		/**
		 * Check if a tile allows looking through (windows, certain doors).
		 */
		bool IsLookPossible(const Tile* tile) const;

	private:
		static constexpr int AWARE_UNDERGROUND_FLOOR_RANGE = 2;
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_FLOOR_VISIBILITY_CALCULATOR_H_
