#include "app/main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/overlays/preview_drawer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "rendering/ui/map_display.h"
#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "ui/gui.h"
#include "brushes/brush.h"
#include "editor/copybuffer.h"

PreviewDrawer::PreviewDrawer() {
}

PreviewDrawer::~PreviewDrawer() {
}

void PreviewDrawer::draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, MapCanvas* canvas, const RenderView& view, int map_z, const DrawingOptions& options, Editor& editor, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, uint32_t current_house_id) {
	if (g_gui.secondary_map != nullptr && !options.ingame) {
		Brush* brush = g_gui.GetCurrentBrush();

		Position normalPos;
		Position to(view.mouse_map_x, view.mouse_map_y, view.floor);

		if (canvas->isPasting()) {
			normalPos = editor.copybuffer.getPosition();
		} else if (brush && brush->isDoodad()) {
			normalPos = Position(0x8000, 0x8000, 0x8);
		}

		for (int map_x = view.start_x; map_x <= view.end_x; map_x++) {
			for (int map_y = view.start_y; map_y <= view.end_y; map_y++) {
				Position final(map_x, map_y, map_z);
				Position pos = normalPos + final - to;

				if (pos.z >= MAP_LAYERS || pos.z < 0) {
					continue;
				}

				Tile* tile = g_gui.secondary_map->getTile(pos);
				if (tile) {
					// Compensate for underground/overground
					int offset;
					if (map_z <= GROUND_LAYER) {
						offset = (GROUND_LAYER - map_z) * TileSize;
					} else {
						offset = TileSize * (view.floor - map_z);
					}

					int draw_x = ((map_x * TileSize) - view.view_scroll_x) - offset;
					int draw_y = ((map_y * TileSize) - view.view_scroll_y) - offset;

					// Draw ground
					uint8_t r = 160, g = 160, b = 160;
					if (tile->ground) {
						if (tile->isBlocking() && options.show_blocking) {
							g = g / 3 * 2;
							b = b / 3 * 2;
						}
						if (tile->isHouseTile() && options.show_houses) {
							if ((int)tile->getHouseID() == current_house_id) {
								r /= 2;
							} else {
								r /= 2;
								g /= 2;
							}
						} else if (options.show_special_tiles && tile->isPZ()) {
							r /= 2;
							b /= 2;
						}
						if (options.show_special_tiles && tile->getMapFlags() & TILESTATE_PVPZONE) {
							r = r / 3 * 2;
							b = r / 3 * 2;
						}
						if (options.show_special_tiles && tile->getMapFlags() & TILESTATE_NOLOGOUT) {
							b /= 2;
						}
						if (options.show_special_tiles && tile->getMapFlags() & TILESTATE_NOPVP) {
							g /= 2;
						}
						if (tile->ground) {
							item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, tile->ground, options, true, r, g, b, 160);
						}
					}

					// Draw items on the tile
					if (view.zoom <= 10.0 || !options.hide_items_when_zoomed) {
						ItemVector::iterator it;
						for (it = tile->items.begin(); it != tile->items.end(); it++) {
							if ((*it)->isBorder()) {
								item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, *it, options, true, 160, r, g, b);
							} else {
								item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, *it, options, true, 160, 160, 160, 160);
							}
						}
						if (tile->creature && options.show_creatures) {
							creature_drawer->BlitCreature(sprite_batch, sprite_drawer, draw_x, draw_y, tile->creature);
						}
					}
				}
			}
		}
	}
}
