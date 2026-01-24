//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/tiles/floor_drawer.h"
#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "editor/editor.h"
#include "map/tile.h"

FloorDrawer::FloorDrawer() {
}

FloorDrawer::~FloorDrawer() {
}

void FloorDrawer::draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options, Editor& editor) {

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
							item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, tile->ground, options, false, 128, 255, 128, 96);
						} else {
							item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, tile->ground, options, false, 255, 255, 255, 96);
						}
					}
					if (view.zoom <= 10.0 || !options.hide_items_when_zoomed) {
						ItemVector::iterator it;
						for (it = tile->items.begin(); it != tile->items.end(); it++) {
							item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, *it, options, false, 255, 255, 255, 96);
						}
					}
				}
			}
		}
	}
}
