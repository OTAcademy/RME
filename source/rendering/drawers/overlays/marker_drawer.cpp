#include "app/main.h"
#include "rendering/drawers/overlays/marker_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/core/graphics.h"
#include "rendering/core/sprite_batch.h"
#include "map/tile.h"
#include "game/sprites.h"
#include "editor/editor.h"

MarkerDrawer::MarkerDrawer() {
}

MarkerDrawer::~MarkerDrawer() {
}

void MarkerDrawer::draw(SpriteBatch& sprite_batch, SpriteDrawer* drawer, int draw_x, int draw_y, const Tile* tile, Waypoint* waypoint, uint32_t current_house_id, Editor& editor, const DrawingOptions& options) {
	// waypoint (blue flame)
	if (!options.ingame && waypoint && options.show_waypoints) {
		drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_WAYPOINT, 64, 64, 255);
	}

	// house exit (blue splash)
	if (tile->isHouseExit() && options.show_houses) {
		if (tile->hasHouseExit(current_house_id)) {
			drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_HOUSE_EXIT, 64, 255, 255);
		} else {
			drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_HOUSE_EXIT, 64, 64, 255);
		}
	}

	// town temple (gray flag)
	if (options.show_towns && tile->isTownExit(editor.map)) {
		drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_TOWN_TEMPLE, 255, 255, 64, 170);
	}

	// spawn (purple flame)
	if (tile->spawn && options.show_spawns) {
		if (tile->spawn->isSelected()) {
			drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_SPAWN, 128, 128, 128);
		} else {
			drawer->BlitSprite(sprite_batch, draw_x, draw_y, SPRITE_SPAWN, 255, 255, 255);
		}
	}
}
