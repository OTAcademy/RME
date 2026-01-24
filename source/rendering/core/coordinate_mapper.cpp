//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "rendering/core/coordinate_mapper.h"
#include "app/definitions.h" // For MAP_MAX_LAYER, GROUND_LAYER, TileSize if defined there, or map.h

#ifndef TileSize
	#define TileSize 32
#endif

void CoordinateMapper::ScreenToMap(int screen_x, int screen_y, int view_start_x, int view_start_y, double zoom, int floor, double scale_factor, int* map_x, int* map_y) {
	screen_x = int(screen_x * scale_factor);
	screen_y = int(screen_y * scale_factor);

	if (screen_x < 0) {
		*map_x = (view_start_x + screen_x) / TileSize;
	} else {
		*map_x = int(view_start_x + (screen_x * zoom)) / TileSize;
	}

	if (screen_y < 0) {
		*map_y = (view_start_y + screen_y) / TileSize;
	} else {
		*map_y = int(view_start_y + (screen_y * zoom)) / TileSize;
	}

	if (floor <= GROUND_LAYER) {
		*map_x += GROUND_LAYER - floor;
		*map_y += GROUND_LAYER - floor;
	}
}
