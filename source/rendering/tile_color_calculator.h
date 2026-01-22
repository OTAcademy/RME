#ifndef RME_RENDERING_TILE_COLOR_CALCULATOR_H_
#define RME_RENDERING_TILE_COLOR_CALCULATOR_H_

#include <cstdint>

class Tile;
struct DrawingOptions;

class TileColorCalculator {
public:
	static void Calculate(const Tile* tile, const DrawingOptions& options, uint32_t current_house_id, int spawn_count, uint8_t& r, uint8_t& g, uint8_t& b);
};

#endif
