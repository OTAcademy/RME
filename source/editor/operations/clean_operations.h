#ifndef RME_EDITOR_OPERATIONS_CLEAN_OPERATIONS_H_
#define RME_EDITOR_OPERATIONS_CLEAN_OPERATIONS_H_

#include "map/map.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/materials.h"
#include "ui/gui.h"
#include <algorithm>
#include <limits>

namespace EditorOperations {

	constexpr int PROGRESS_UPDATE_INTERVAL = 0x8000;
	constexpr int PROGRESS_UPDATE_INTERVAL_SMALL = 0x800;
	constexpr int PROGRESS_UPDATE_INTERVAL_UNREACHABLE = 0x1000;
	constexpr int UNREACHABLE_SEARCH_RADIUS_X = 10;
	constexpr int UNREACHABLE_SEARCH_RADIUS_Y = 8;
	constexpr int UNREACHABLE_SEARCH_RADIUS_Z = 2;

	struct RemoveItemCondition {
		RemoveItemCondition(uint16_t itemId) :
			itemId(itemId) { }

		uint16_t itemId;

		bool operator()(Map& map, Item* item, int64_t removed, int64_t done) {
			if (done % PROGRESS_UPDATE_INTERVAL == 0) {
				g_gui.SetLoadDone(static_cast<uint32_t>(100 * done / map.getTileCount()));
			}
			return item->getID() == itemId && !item->isComplex();
		}
	};

	struct RemoveCorpsesCondition {
		RemoveCorpsesCondition() { }

		bool operator()(Map& map, Item* item, long long removed, long long done) {
			if (done % PROGRESS_UPDATE_INTERVAL_SMALL == 0) {
				g_gui.SetLoadDone(static_cast<uint32_t>(100 * done / map.getTileCount()));
			}

			return g_materials.isInTileset(item, "Corpses") && !item->isComplex();
		}
	};

	struct RemoveUnreachableCondition {
		RemoveUnreachableCondition() { }

		bool isReachable(Tile* tile) {
			if (tile == nullptr) {
				return false;
			}
			if (!tile->isBlocking()) {
				return true;
			}
			return false;
		}

		bool operator()(Map& map, Tile* tile, long long removed, long long done, long long total) {
			if (done % PROGRESS_UPDATE_INTERVAL_UNREACHABLE == 0) {
				g_gui.SetLoadDone(static_cast<uint32_t>(100 * done / total));
			}

			Position pos = tile->getPosition();
			int sx = std::max(pos.x - UNREACHABLE_SEARCH_RADIUS_X, 0);
			int ex = std::min(pos.x + UNREACHABLE_SEARCH_RADIUS_X, (int)std::numeric_limits<uint16_t>::max());
			int sy = std::max(pos.y - UNREACHABLE_SEARCH_RADIUS_Y, 0);
			int ey = std::min(pos.y + UNREACHABLE_SEARCH_RADIUS_Y, (int)std::numeric_limits<uint16_t>::max());
			int sz, ez;

			if (pos.z <= GROUND_LAYER) {
				sz = 0;
				ez = 9;
			} else {
				// underground
				sz = std::max(pos.z - UNREACHABLE_SEARCH_RADIUS_Z, GROUND_LAYER);
				ez = std::min(pos.z + UNREACHABLE_SEARCH_RADIUS_Z, MAP_MAX_LAYER);
			}

			for (int z = sz; z <= ez; ++z) {
				for (int y = sy; y <= ey; ++y) {
					for (int x = sx; x <= ex; ++x) {
						if (isReachable(map.getTile(x, y, z))) {
							return false;
						}
					}
				}
			}
			return true;
		}
	};

}

#endif
