#ifndef RME_MARKER_DRAWER_H_
#define RME_MARKER_DRAWER_H_

#include "rendering/drawing_options.h"
#include <cstdint>

class SpriteDrawer;
class Tile;
class Waypoint;
class Editor;

class MarkerDrawer {
public:
	MarkerDrawer();
	~MarkerDrawer();

	void draw(SpriteDrawer* drawer, int draw_x, int draw_y, const Tile* tile, Waypoint* waypoint, uint32_t current_house_id, Editor& editor, const DrawingOptions& options);
};

#endif
