#ifndef RME_MAP_STATISTICS_H_
#define RME_MAP_STATISTICS_H_

#include "app/main.h"
#include <string>
#include <map>

class Map;
class Town;
class House;

struct MapStatistics {
	uint64_t tile_count = 0;
	uint64_t detailed_tile_count = 0;
	uint64_t blocking_tile_count = 0;
	uint64_t walkable_tile_count = 0;
	double percent_pathable = 0.0;
	double percent_detailed = 0.0;
	uint64_t spawn_count = 0;
	uint64_t creature_count = 0;
	double creatures_per_spawn = 0.0;

	uint64_t item_count = 0;
	uint64_t loose_item_count = 0;
	uint64_t depot_count = 0;
	uint64_t action_item_count = 0;
	uint64_t unique_item_count = 0;
	uint64_t container_count = 0;

	int town_count = 0;
	int house_count = 0;
	uint64_t total_house_sqm = 0;
	const Town* largest_town = nullptr;
	uint64_t largest_town_size = 0;
	const House* largest_house = nullptr;
	uint64_t largest_house_size = 0;
	double houses_per_town = 0.0;
	double sqm_per_house = 0.0;
	double sqm_per_town = 0.0;
};

class MapStatisticsCollector {
public:
	static MapStatistics Collect(Map* map);
};

#endif
