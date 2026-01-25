#include "app/main.h"
#include "map/map_search.h"
#include "map/map.h"
#include "map/tile.h"
#include "ui/gui.h"
#include "editor/editor.h"
#include "editor/operations/search_operations.h"
#include <algorithm>

std::vector<SearchResult> MapSearchUtility::SearchItems(Map& map, bool unique, bool action, bool container, bool writable, bool onSelection) {
	EditorOperations::MapSearcher searcher;
	searcher.search_unique = unique;
	searcher.search_action = action;
	searcher.search_container = container;
	searcher.search_writeable = writable;

	foreach_ItemOnMap(map, searcher, onSelection);
	searcher.sort();

	std::vector<SearchResult> results;
	for (auto& p : searcher.found) {
		results.push_back({ p.first, p.second, std::string(searcher.desc(p.second).c_str()) });
	}
	return results;
}
