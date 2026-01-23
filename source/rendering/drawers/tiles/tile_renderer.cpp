#include "main.h"
// Force recompile
#include "rendering/drawers/tiles/tile_renderer.h"

#include "editor.h"
#include "tile.h"
#include "item.h"
#include "items.h"
#include "waypoint_brush.h"

#include "rendering/core/drawing_options.h"
#include "rendering/core/render_view.h"
#include "rendering/drawers/tiles/tile_color_calculator.h"
#include "definitions.h"
#include "sprites.h"

#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/tiles/floor_drawer.h"
#include "rendering/drawers/overlays/marker_drawer.h"
#include "rendering/ui/tooltip_drawer.h"
#include "rendering/core/light_buffer.h"

TileRenderer::TileRenderer(ItemDrawer* id, SpriteDrawer* sd, CreatureDrawer* cd, FloorDrawer* fd, MarkerDrawer* md, TooltipDrawer* td, Editor* ed) : item_drawer(id), sprite_drawer(sd), creature_drawer(cd), floor_drawer(fd), marker_drawer(md), tooltip_drawer(td), editor(ed) {
}

void TileRenderer::DrawTile(TileLocation* location, const RenderView& view, const DrawingOptions& options, uint32_t current_house_id, std::ostringstream& tooltip_stream) {
	if (!location) {
		return;
	}
	Tile* tile = location->get();

	if (!tile) {
		return;
	}

	if (options.show_only_modified && !tile->isModified()) {
		return;
	}

	int map_x = location->getX();
	int map_y = location->getY();
	int map_z = location->getZ();

	// Early viewport culling - skip tiles that are completely off-screen
	if (!view.IsTileVisible(map_x, map_y, map_z)) {
		return;
	}

	int draw_x, draw_y;
	view.getScreenPosition(map_x, map_y, map_z, draw_x, draw_y);

	Waypoint* waypoint = editor->map.waypoints.getWaypoint(location);
	if (options.show_tooltips && location->getWaypointCount() > 0) {
		if (waypoint) {
			tooltip_drawer->writeTooltip(waypoint, tooltip_stream);
		}
	}

	bool as_minimap = options.show_as_minimap;
	bool only_colors = as_minimap || options.show_only_colors;

	uint8_t r = 255, g = 255, b = 255;

	// begin filters for ground tile
	// begin filters for ground tile
	if (!as_minimap) {
		TileColorCalculator::Calculate(tile, options, current_house_id, location->getSpawnCount(), r, g, b);
	}

	if (only_colors) {
		if (as_minimap) {
			TileColorCalculator::GetMinimapColor(tile, r, g, b);
			sprite_drawer->glBlitSquare(draw_x, draw_y, r, g, b, 255);
		} else if (r != 255 || g != 255 || b != 255) {
			sprite_drawer->glBlitSquare(draw_x, draw_y, r, g, b, 128);
		}
	} else {
		if (tile->ground) {
			if (options.show_preview && view.zoom <= 2.0) {
				tile->ground->animate();
			}

			item_drawer->BlitItem(sprite_drawer, creature_drawer, draw_x, draw_y, tile, tile->ground, options, false, r, g, b);
		} else if (options.always_show_zones && (r != 255 || g != 255 || b != 255)) {
			ItemType* zoneItem = &g_items[SPRITE_ZONE];
			item_drawer->DrawRawBrush(sprite_drawer, draw_x, draw_y, zoneItem, r, g, b, 60);
		}
	}

	if (options.show_tooltips && map_z == view.floor && tile->ground) {
		tooltip_drawer->writeTooltip(tile->ground, tooltip_stream, false);
	}

	// end filters for ground tile

	if (!only_colors) {
		if (view.zoom < 10.0 || !options.hide_items_when_zoomed) {
			// items on tile
			for (ItemVector::iterator it = tile->items.begin(); it != tile->items.end(); it++) {
				// item tooltip
				if (options.show_tooltips && map_z == view.floor) {
					tooltip_drawer->writeTooltip(*it, tooltip_stream, tile->isHouseTile());
				}

				// item animation
				if (options.show_preview && view.zoom <= 2.0) {
					(*it)->animate();
				}

				// item sprite
				if ((*it)->isBorder()) {
					item_drawer->BlitItem(sprite_drawer, creature_drawer, draw_x, draw_y, tile, *it, options, false, r, g, b);
				} else {
					uint8_t ir = 255, ig = 255, ib = 255;

					if (options.extended_house_shader && options.show_houses && tile->isHouseTile()) {
						if ((int)tile->getHouseID() == current_house_id) {
							ir /= 2;
						} else {
							ir /= 2;
							ig /= 2;
						}
					}
					item_drawer->BlitItem(sprite_drawer, creature_drawer, draw_x, draw_y, tile, *it, options, false, ir, ig, ib);
				}
			}
			// monster/npc on tile
			if (tile->creature && options.show_creatures) {
				creature_drawer->BlitCreature(sprite_drawer, draw_x, draw_y, tile->creature);
			}
		}

		if (view.zoom < 10.0) {
			// markers (waypoint, house exit, town temple, spawn)
			marker_drawer->draw(sprite_drawer, draw_x, draw_y, tile, waypoint, current_house_id, *editor, options);

			// tooltips
			if (options.show_tooltips) {
				if (location->getWaypointCount() > 0) {
					tooltip_drawer->addTooltip(draw_x, draw_y, tooltip_stream.str(), 0, 255, 0);
				} else {
					tooltip_drawer->addTooltip(draw_x, draw_y, tooltip_stream.str());
				}
			}

			tooltip_stream.str("");
		}
	}
}

void TileRenderer::AddLight(TileLocation* location, const RenderView& view, const DrawingOptions& options, LightBuffer& light_buffer) {
	if (!options.isDrawLight() || !location) {
		return;
	}

	auto tile = location->get();
	if (!tile) {
		return;
	}

	const auto& position = location->getPosition();

	if (tile->ground) {
		if (tile->ground->hasLight()) {
			light_buffer.AddLight(position.x, position.y, position.z, tile->ground->getLight());
		}
	}

	bool hidden = options.hide_items_when_zoomed && view.zoom > 10.f;
	if (!hidden && !tile->items.empty()) {
		for (auto item : tile->items) {
			if (item->hasLight()) {
				light_buffer.AddLight(position.x, position.y, position.z, item->getLight());
			}
		}
	}
}
