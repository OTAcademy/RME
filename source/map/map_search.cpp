#include "app/main.h"
#include "map/map_search.h"
#include "map/map.h"
#include "map/tile.h"
#include "ui/gui.h"
#include "editor/editor.h"
#include <algorithm>

namespace {
	struct Searcher {
		Searcher() :
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
			if (done % 0x8000 == 0) {
				g_gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}
			Container* container;
			if ((search_unique && item->getUniqueID() > 0) || (search_action && item->getActionID() > 0) || (search_container && ((container = dynamic_cast<Container*>(item)) && container->getItemCount())) || (search_writeable && item->getText().length() > 0)) {
				found.push_back(std::make_pair(tile, item));
			}
		}

		std::string desc(Item* item) {
			std::string label;
			if (search_action) {
				if (item->getActionID() > 0) {
					label += "AID:" + std::to_string(item->getActionID()) + " ";
				}
				if (item->getUniqueID() > 0) {
					label += "UID:" + std::to_string(item->getUniqueID()) + " ";
				}
			} else {
				if (item->getUniqueID() > 0) {
					label += "UID:" + std::to_string(item->getUniqueID()) + " ";
				}
				if (item->getActionID() > 0) {
					label += "AID:" + std::to_string(item->getActionID()) + " ";
				}
			}

			label += item->getName();

			if (dynamic_cast<Container*>(item)) {
				label += " (Container) ";
			}

			if (item->getText().length() > 0) {
				label += " (Text: " + item->getText() + ") ";
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
				std::sort(found.begin(), found.end(), compare);
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

std::vector<SearchResult> MapSearchUtility::SearchItems(Map& map, bool unique, bool action, bool container, bool writable, bool onSelection) {
	Searcher searcher;
	searcher.search_unique = unique;
	searcher.search_action = action;
	searcher.search_container = container;
	searcher.search_writeable = writable;

	foreach_ItemOnMap(map, searcher, onSelection);
	searcher.sort();

	std::vector<SearchResult> results;
	for (auto& p : searcher.found) {
		results.push_back({ p.first, p.second, searcher.desc(p.second) });
	}
	return results;
}
