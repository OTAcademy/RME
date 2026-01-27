#ifndef RME_EDITOR_OPERATIONS_SEARCH_OPERATIONS_H_
#define RME_EDITOR_OPERATIONS_SEARCH_OPERATIONS_H_

#include "map/map.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/complexitem.h"
#include "ui/gui.h"

namespace EditorOperations {

	constexpr int SEARCH_UPDATE_INTERVAL = 0x8000;

	struct ItemSearcher {
		ItemSearcher(uint16_t itemId, uint32_t maxCount) :
			itemId(itemId), maxCount(maxCount) { }

		uint16_t itemId;
		uint32_t maxCount;
		std::vector<std::pair<Tile*, Item*>> result;

		bool limitReached() const {
			return result.size() >= (size_t)maxCount;
		}

		void operator()(Map& map, Tile* tile, Item* item, long long done) {
			if (result.size() >= (size_t)maxCount) {
				return;
			}

			if (done % SEARCH_UPDATE_INTERVAL == 0) {
				g_gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}

			if (item->getID() == itemId) {
				result.push_back(std::make_pair(tile, item));
			}
		}
	};

	struct MapSearcher {
		MapSearcher() :
			search_unique(false),
			search_action(false),
			search_container(false),
			search_writeable(false) { }

		bool search_unique;
		bool search_action;
		bool search_container;
		bool search_writeable;
		std::vector<std::pair<Tile*, Item*>> found;

		void operator()(Map& map, Tile* tile, Item* item, long long done) {
			if (done % SEARCH_UPDATE_INTERVAL == 0) {
				g_gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}
			Container* container;
			if ((search_unique && item->getUniqueID() > 0) || (search_action && item->getActionID() > 0) || (search_container && ((container = dynamic_cast<Container*>(item)) && container->getItemCount())) || (search_writeable && item->getText().length() > 0)) {
				found.push_back(std::make_pair(tile, item));
			}
		}

		wxString desc(Item* item) {
			wxString label;
			if (search_action) {
				if (item->getActionID() > 0) {
					label << "AID:" << item->getActionID() << " ";
				}
				if (item->getUniqueID() > 0) {
					label << "UID:" << item->getUniqueID() << " ";
				}
			} else {
				if (item->getUniqueID() > 0) {
					label << "UID:" << item->getUniqueID() << " ";
				}
				if (item->getActionID() > 0) {
					label << "AID:" << item->getActionID() << " ";
				}
			}

			label << wxstr(std::string(item->getName()));

			if (dynamic_cast<Container*>(item)) {
				label << " (Container) ";
			}

			if (item->getText().length() > 0) {
				label << " (Text: " << wxstr(std::string(item->getText())) << ") ";
			}

			return label;
		}

		void sort() {
			if (search_unique && !search_action) {
				std::sort(found.begin(), found.end(), [](const std::pair<Tile*, Item*>& pair1, const std::pair<Tile*, Item*>& pair2) {
					const Item* item1 = pair1.second;
					const Item* item2 = pair2.second;

					uint16_t u1 = item1->getUniqueID();
					uint16_t u2 = item2->getUniqueID();
					if (u1 != u2) {
						return u1 < u2;
					}
					return item1->getActionID() < item2->getActionID();
				});
			} else if (search_action && !search_unique) {
				std::sort(found.begin(), found.end(), [](const std::pair<Tile*, Item*>& pair1, const std::pair<Tile*, Item*>& pair2) {
					const Item* item1 = pair1.second;
					const Item* item2 = pair2.second;

					uint16_t a1 = item1->getActionID();
					uint16_t a2 = item2->getActionID();
					if (a1 != a2) {
						return a1 < a2;
					}
					return item1->getUniqueID() < item2->getUniqueID();
				});
			} else if (search_unique || search_action) {
				std::sort(found.begin(), found.end(), MapSearcher::compare);
			}
		}

		static bool compare(const std::pair<Tile*, Item*>& pair1, const std::pair<Tile*, Item*>& pair2) {
			const Item* item1 = pair1.second;
			const Item* item2 = pair2.second;

			if (item1->getActionID() != 0 || item2->getActionID() != 0) {
				return item1->getActionID() < item2->getActionID();
			} else if (item1->getUniqueID() != 0 || item2->getUniqueID() != 0) {
				return item1->getUniqueID() < item2->getUniqueID();
			}

			return false;
		}
	};
}

#endif
