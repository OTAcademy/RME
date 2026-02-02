//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

// glut include removed

#include "rendering/drawers/cursors/drag_shadow_drawer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/map_drawer.h"
#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "editor/editor.h"
#include "rendering/ui/map_display.h"
#include "map/tile.h"
#include "game/sprites.h"

#include "game/item.h"
#include "game/creature.h"
#include "game/spawn.h"
#include "rendering/ui/selection_controller.h"

DragShadowDrawer::DragShadowDrawer() {
}

DragShadowDrawer::~DragShadowDrawer() {
}

#include "rendering/core/primitive_renderer.h"

void DragShadowDrawer::draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, MapDrawer* drawer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options) {
	if (!drawer || !drawer->canvas) {
		return;
	}

	// Draw dragging shadow
	if (!drawer->editor.selection.isBusy() && options.dragging && !options.ingame) {
		for (auto tit = drawer->editor.selection.begin(); tit != drawer->editor.selection.end(); tit++) {
			Tile* tile = *tit;
			Position pos = tile->getPosition();

			int move_x, move_y, move_z;
			Position drag_start = drawer->canvas->selection_controller->GetDragStartPosition();
			move_x = drag_start.x - view.mouse_map_x;
			move_y = drag_start.y - view.mouse_map_y;
			move_z = drag_start.z - view.floor;

			pos.x -= move_x;
			pos.y -= move_y;
			pos.z -= move_z;

			if (pos.z < 0 || pos.z >= MAP_LAYERS) {
				continue;
			}

			// On screen and dragging?
			if (pos.x + 2 > view.start_x && pos.x < view.end_x && pos.y + 2 > view.start_y && pos.y < view.end_y && (move_x != 0 || move_y != 0 || move_z != 0)) {
				int offset;
				if (pos.z <= GROUND_LAYER) {
					offset = (GROUND_LAYER - pos.z) * TileSize;
				} else {
					offset = TileSize * (view.floor - pos.z);
				}

				int draw_x = ((pos.x * TileSize) - view.view_scroll_x) - offset;
				int draw_y = ((pos.y * TileSize) - view.view_scroll_y) - offset;

				// save performance when moving large chunks unzoomed
				ItemVector toRender = tile->getSelectedItems(view.zoom > 3.0);
				Tile* desttile = drawer->editor.map.getTile(pos);
				for (ItemVector::const_iterator iit = toRender.begin(); iit != toRender.end(); iit++) {
					if (desttile) {
						item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, desttile, *iit, options, true, 160, 160, 160, 160);
					} else {
						item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, pos, *iit, options, true, 160, 160, 160, 160);
					}
				}

				// save performance when moving large chunks unzoomed
				if (view.zoom <= 3.0) {
					if (tile->creature && tile->creature->isSelected() && options.show_creatures) {
						creature_drawer->BlitCreature(sprite_batch, sprite_drawer, draw_x, draw_y, tile->creature);
					}
					if (tile->spawn && tile->spawn->isSelected()) {
						sprite_drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_SPAWN, 160, 160, 160, 160);
					}
				}
			}
		}
	}
}
