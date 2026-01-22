//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/floor_drawer.h"
#include "rendering/map_drawer.h"
#include "rendering/render_view.h"
#include "rendering/drawing_options.h"
#include "editor.h"
#include "tile.h"

FloorDrawer::FloorDrawer() {
}

FloorDrawer::~FloorDrawer() {
}

void FloorDrawer::draw(MapDrawer* drawer, const RenderView& view, const DrawingOptions& options, Editor& editor) {
	if (!drawer) {
		return;
	}

	glEnable(GL_TEXTURE_2D);

	// Draw "transparent higher floor"
	if (view.floor != 8 && view.floor != 0 && options.transparent_floors) {
		int map_z = view.floor - 1;
		for (int map_x = view.start_x; map_x <= view.end_x; map_x++) {
			for (int map_y = view.start_y; map_y <= view.end_y; map_y++) {
				Tile* tile = editor.map.getTile(map_x, map_y, map_z);
				if (tile) {
					int offset;
					if (map_z <= GROUND_LAYER) {
						offset = (GROUND_LAYER - map_z) * TileSize;
					} else {
						offset = TileSize * (view.floor - map_z);
					}

					int draw_x = ((map_x * TileSize) - view.view_scroll_x) - offset;
					int draw_y = ((map_y * TileSize) - view.view_scroll_y) - offset;

					// Position pos = tile->getPosition();

					if (tile->ground) {
						if (tile->isPZ()) {
							drawer->BlitItem(draw_x, draw_y, tile, tile->ground, false, 128, 255, 128, 96);
						} else {
							drawer->BlitItem(draw_x, draw_y, tile, tile->ground, false, 255, 255, 255, 96);
						}
					}
					if (view.zoom <= 10.0 || !options.hide_items_when_zoomed) {
						ItemVector::iterator it;
						for (it = tile->items.begin(); it != tile->items.end(); it++) {
							drawer->BlitItem(draw_x, draw_y, tile, *it, false, 255, 255, 255, 96);
						}
					}
				}
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}
