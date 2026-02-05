#include "app/main.h"
#include "map/map_statistics.h"
#include "map/map.h"
#include "map/tile.h"
#include "game/items.h"
#include "ui/gui.h"
#include <sstream>

MapStatistics MapStatisticsCollector::Collect(Map* map) {
	MapStatistics stats;
	int load_counter = 0;

	std::map<uint32_t, uint32_t> town_sqm_count;

	for (auto& tile_location : *map) {
		Tile* tile = tile_location.get();
		if (load_counter % 8192 == 0) {
			g_gui.SetLoadDone((unsigned int)(int64_t(load_counter) * 95ll / int64_t(map->getTileCount())));
		}

		if (tile->empty()) {
			continue;
		}

		stats.tile_count += 1;

		bool is_detailed = false;
		auto analyze_item = [&](Item* item) {
			stats.item_count += 1;
			if (!item->isGroundTile() && !item->isBorder()) {
				is_detailed = true;
				ItemType& it = g_items[item->getID()];
				if (it.moveable) {
					stats.loose_item_count += 1;
				}
				if (it.isDepot()) {
					stats.depot_count += 1;
				}
				if (item->getActionID() > 0) {
					stats.action_item_count += 1;
				}
				if (item->getUniqueID() > 0) {
					stats.unique_item_count += 1;
				}
				if (Container* c = dynamic_cast<Container*>(item)) {
					if (c->getVector().size()) {
						stats.container_count += 1;
					}
				}
			}
		};

		if (tile->ground) {
			analyze_item(tile->ground);
		}

		for (Item* item : tile->items) {
			analyze_item(item);
		}

		if (tile->spawn) {
			stats.spawn_count += 1;
		}

		if (tile->creature) {
			stats.creature_count += 1;
		}

		if (tile->isBlocking()) {
			stats.blocking_tile_count += 1;
		} else {
			stats.walkable_tile_count += 1;
		}

		if (is_detailed) {
			stats.detailed_tile_count += 1;
		}

		load_counter += 1;
	}

	stats.creatures_per_spawn = (stats.spawn_count != 0 ? double(stats.creature_count) / double(stats.spawn_count) : -1.0);
	stats.percent_pathable = 100.0 * (stats.tile_count != 0 ? double(stats.walkable_tile_count) / double(stats.tile_count) : -1.0);
	stats.percent_detailed = 100.0 * (stats.tile_count != 0 ? double(stats.detailed_tile_count) / double(stats.tile_count) : -1.0);

	load_counter = 0;
	stats.town_count = map->towns.count();
	stats.house_count = map->houses.count();

	Houses& houses = map->houses;
	for (const auto& [house_id, house] : houses) {

		if (load_counter % 64) {
			g_gui.SetLoadDone((unsigned int)(95ll + int64_t(load_counter) * 5ll / int64_t(stats.house_count)));
		}

		if (house->size() > stats.largest_house_size) {
			stats.largest_house = house.get();
			stats.largest_house_size = house->size();
		}
		stats.total_house_sqm += house->size();
		town_sqm_count[house->townid] += house->size();
		load_counter++;
	}

	stats.houses_per_town = (stats.town_count != 0 ? double(stats.house_count) / double(stats.town_count) : -1.0);
	stats.sqm_per_house = (stats.house_count != 0 ? double(stats.total_house_sqm) / double(stats.house_count) : -1.0);
	stats.sqm_per_town = (stats.town_count != 0 ? double(stats.total_house_sqm) / double(stats.town_count) : -1.0);

	Towns& towns = map->towns;
	for (const auto& [town_id, town_sqm] : town_sqm_count) {
		Town* town = towns.getTown(town_id);
		if (town && town_sqm > stats.largest_town_size) {
			stats.largest_town = town;
			stats.largest_town_size = town_sqm;
		}
	}

	return stats;
}
