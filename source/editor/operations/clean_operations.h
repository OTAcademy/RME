#ifndef RME_EDITOR_OPERATIONS_CLEAN_OPERATIONS_H_
#define RME_EDITOR_OPERATIONS_CLEAN_OPERATIONS_H_

#include "map/map.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/materials.h"
#include "ui/gui.h"
#include <algorithm>

namespace EditorOperations {

	struct RemoveItemCondition {
		RemoveItemCondition(uint16_t itemId) :
			itemId(itemId) { }

		uint16_t itemId;

		bool operator()(Map& map, Item* item, int64_t removed, int64_t done) {
			if (done % 0x8000 == 0) {
				g_gui.SetLoadDone((uint32_t)(100 * done / map.getTileCount()));
			}
			return item->getID() == itemId && !item->isComplex();
		}
	};

	struct RemoveCorpsesCondition {
		RemoveCorpsesCondition() { }

		bool operator()(Map& map, Item* item, long long removed, long long done) {
			if (done % 0x800 == 0) {
				g_gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}

			return g_materials.isInTileset(item, "Corpses") & !item->isComplex();
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
			if (done % 0x1000 == 0) {
				g_gui.SetLoadDone((unsigned int)(100 * done / total));
			}

			Position pos = tile->getPosition();
			int sx = std::max(pos.x - 10, 0);
			int ex = std::min(pos.x + 10, 65535);
			int sy = std::max(pos.y - 8, 0);
			int ey = std::min(pos.y + 8, 65535);
			int sz, ez;

			if (pos.z <= GROUND_LAYER) {
				sz = 0;
				ez = 9;
			} else {
				// underground
				sz = std::max(pos.z - 2, GROUND_LAYER);
				ez = std::min(pos.z + 2, MAP_MAX_LAYER);
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
