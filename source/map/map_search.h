#ifndef RME_MAP_SEARCH_H_
#define RME_MAP_SEARCH_H_

#include "app/main.h"
#include <vector>
#include <utility>
#include <string>

class Map;
class Tile;
class Item;

struct SearchResult {
	Tile* tile;
	Item* item;
	std::string description;
};

class MapSearchUtility {
public:
	static std::vector<SearchResult> SearchItems(Map& map, bool unique, bool action, bool container, bool writable, bool onSelection);
};

#endif
