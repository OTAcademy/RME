#include "app/main.h"
#include "rendering/drawers/tiles/tile_renderer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "ui/gui.h"

#include "editor/editor.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/items.h"
#include "brushes/waypoint/waypoint_brush.h"
#include "game/complexitem.h"

#include "rendering/core/drawing_options.h"
#include "rendering/core/render_view.h"
#include "rendering/drawers/tiles/tile_color_calculator.h"
#include "app/definitions.h"
#include "game/sprites.h"

#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/entities/creature_name_drawer.h"
#include "rendering/drawers/tiles/floor_drawer.h"
#include "rendering/drawers/overlays/marker_drawer.h"
#include "rendering/ui/tooltip_drawer.h"
#include "rendering/core/light_buffer.h"

TileRenderer::TileRenderer(ItemDrawer* id, SpriteDrawer* sd, CreatureDrawer* cd, CreatureNameDrawer* cnd, FloorDrawer* fd, MarkerDrawer* md, TooltipDrawer* td, Editor* ed) :
	item_drawer(id), sprite_drawer(sd), creature_drawer(cd), creature_name_drawer(cnd), floor_drawer(fd), marker_drawer(md), tooltip_drawer(td), editor(ed) {
}

// Helper function to populate tooltip data from an item (in-place)
static bool FillItemTooltipData(TooltipData& data, Item* item, const Position& pos, bool isHouseTile) {
	if (!item) {
		return false;
	}

	const uint16_t id = item->getID();
	if (id < 100) {
		return false;
	}

	const uint16_t unique = item->getUniqueID();
	const uint16_t action = item->getActionID();
	std::string_view text = item->getText();
	std::string_view description = item->getDescription();
	uint8_t doorId = 0;
	Position destination;

	// Check if it's a door
	if (isHouseTile && item->isDoor()) {
		if (const Door* door = item->asDoor()) {
			if (door->isRealDoor()) {
				doorId = door->getDoorID();
			}
		}
	}

	// Check if it's a teleport
	if (item->isTeleport()) {
		Teleport* tp = static_cast<Teleport*>(item);
		if (tp->hasDestination()) {
			destination = tp->getDestination();
		}
	}

	// Check if container has content
	bool hasContent = false;
	if (g_items[id].isContainer()) {
		if (const Container* container = item->asContainer()) {
			hasContent = container->getItemCount() > 0;
		}
	}

	// Only create tooltip if there's something to show
	if (unique == 0 && action == 0 && doorId == 0 && text.empty() && description.empty() && destination.x == 0 && !hasContent) {
		return false;
	}

	// Get item name from database
	std::string_view itemName = g_items[id].name;
	if (itemName.empty()) {
		itemName = "Item";
	}

	data.pos = pos;
	data.itemId = id;
	data.itemName = itemName; // Assign string_view to string_view (no copy)

	data.actionId = action;
	data.uniqueId = unique;
	data.doorId = doorId;
	data.text = text;
	data.description = description;
	data.destination = destination;

	// Populate container items
	if (g_items[id].isContainer()) {
		if (const Container* container = item->asContainer()) {
			// Set capacity for rendering empty slots
			data.containerCapacity = static_cast<uint8_t>(container->getVolume());

			const ItemVector& items = container->getVector();
			data.containerItems.clear();
			data.containerItems.reserve(items.size());
			for (Item* subItem : items) {
				if (subItem) {
					ContainerItem ci;
					ci.id = subItem->getID();
					ci.subtype = subItem->getSubtype();
					ci.count = subItem->getCount();
					// Sanity check for count
					if (ci.count == 0) {
						ci.count = 1;
					}

					data.containerItems.push_back(ci);

					// Limit preview items to avoid massive tooltips
					if (data.containerItems.size() >= 32) {
						break;
					}
				}
			}
		}
	}

	data.updateCategory();

	return true;
}

void TileRenderer::DrawTile(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, TileLocation* location, const RenderView& view, const DrawingOptions& options, uint32_t current_house_id, int in_draw_x, int in_draw_y) {
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

	int draw_x, draw_y;
	if (in_draw_x != -1 && in_draw_y != -1) {
		draw_x = in_draw_x;
		draw_y = in_draw_y;
	} else {
		// Early viewport culling - skip tiles that are completely off-screen
		if (!view.IsTileVisible(map_x, map_y, map_z, draw_x, draw_y)) {
			return;
		}
	}

	Waypoint* waypoint = editor->map.waypoints.getWaypoint(location);

	// Waypoint tooltip (one per waypoint)
	if (options.show_tooltips && location->getWaypointCount() > 0 && waypoint && map_z == view.floor) {
		tooltip_drawer->addWaypointTooltip(location->getPosition(), waypoint->name);
	}

	bool as_minimap = options.show_as_minimap;
	bool only_colors = as_minimap || options.show_only_colors;

	uint8_t r = 255, g = 255, b = 255;

	// begin filters for ground tile
	if (!as_minimap) {
		TileColorCalculator::Calculate(tile, options, current_house_id, location->getSpawnCount(), r, g, b);
	}

	if (only_colors) {
		if (as_minimap) {
			TileColorCalculator::GetMinimapColor(tile, r, g, b);
			sprite_drawer->glBlitSquare(sprite_batch, draw_x, draw_y, r, g, b, 255);
		} else if (r != 255 || g != 255 || b != 255) {
			sprite_drawer->glBlitSquare(sprite_batch, draw_x, draw_y, r, g, b, 128);
		}
	} else {
		if (tile->ground) {
			if (options.show_preview && view.zoom <= 2.0) {
				tile->ground->animate();
			}

			item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, tile->ground, options, false, r, g, b);
		} else if (options.always_show_zones && (r != 255 || g != 255 || b != 255)) {
			ItemType* zoneItem = &g_items[SPRITE_ZONE];
			item_drawer->DrawRawBrush(sprite_batch, sprite_drawer, draw_x, draw_y, zoneItem, r, g, b, 60);
		}
	}

	// Ground tooltip (one per item)
	if (options.show_tooltips && map_z == view.floor && tile->ground) {
		TooltipData& groundData = tooltip_drawer->requestTooltipData();
		if (FillItemTooltipData(groundData, tile->ground, location->getPosition(), tile->isHouseTile())) {
			if (groundData.hasVisibleFields()) {
				tooltip_drawer->commitTooltip();
			}
		}
	}

	// end filters for ground tile

	// Draw helper border for selected house tiles
	// Only draw on the current floor (grid)
	if (options.show_houses && tile->isHouseTile() && static_cast<int>(tile->getHouseID()) == current_house_id && map_z == view.floor) {

		uint8_t hr, hg, hb;
		TileColorCalculator::GetHouseColor(tile->getHouseID(), hr, hg, hb);

		float intensity = 0.5f + (0.5f * options.highlight_pulse);
		glm::vec4 border_color(static_cast<float>(hr) / 255.0f, static_cast<float>(hg) / 255.0f, static_cast<float>(hb) / 255.0f, intensity); // House color border with pulsing alpha

		// Map coordinates to screen coordinates
		// draw_x, draw_y are defined in the beginning of function and are top-left of the tile
		// Draw 1px solid border using geometry generation
		// primitive_renderer.drawBox(glm::vec4(x, y, s, s), border_color, 1.0f);

		// Use SpriteDrawer to keep batching unified (prevents PrimitiveRenderer flush/state change)
		int br = static_cast<int>(border_color.r * 255.0f);
		int bg = static_cast<int>(border_color.g * 255.0f);
		int bb = static_cast<int>(border_color.b * 255.0f);
		int ba = static_cast<int>(border_color.a * 255.0f);
		sprite_drawer->glDrawBox(sprite_batch, draw_x, draw_y, 32, 32, br, bg, bb, ba);
	}

	if (!only_colors) {
		if (view.zoom < 10.0 || !options.hide_items_when_zoomed) {
			// Hoist house color calculation out of item loop
			uint8_t house_r = 255, house_g = 255, house_b = 255;
			bool calculate_house_color = options.extended_house_shader && options.show_houses && tile->isHouseTile();
			if (calculate_house_color) {
				TileColorCalculator::GetHouseColor(tile->getHouseID(), house_r, house_g, house_b);
			}

			// items on tile
			for (auto* item : tile->items) {
				// item tooltip (one per item)
				if (options.show_tooltips && map_z == view.floor) {
					TooltipData& itemData = tooltip_drawer->requestTooltipData();
					if (FillItemTooltipData(itemData, item, location->getPosition(), tile->isHouseTile())) {
						if (itemData.hasVisibleFields()) {
							tooltip_drawer->commitTooltip();
						}
					}
				}

				// item animation
				if (options.show_preview && view.zoom <= 2.0) {
					item->animate();
				}

				// item sprite
				if (item->isBorder()) {
					item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, item, options, false, r, g, b);
				} else {
					uint8_t ir = 255, ig = 255, ib = 255;

					if (calculate_house_color) {
						// Apply house color tint
						ir = static_cast<uint8_t>(ir * house_r / 255);
						ig = static_cast<uint8_t>(ig * house_g / 255);
						ib = static_cast<uint8_t>(ib * house_b / 255);

						if (static_cast<int>(tile->getHouseID()) == current_house_id) {
							// Pulse effect matching the tile pulse
							if (options.highlight_pulse > 0.0f) {
								float boost = options.highlight_pulse * 0.6f;
								ir = static_cast<uint8_t>(std::min(255, static_cast<int>(ir + (255 - ir) * boost)));
								ig = static_cast<uint8_t>(std::min(255, static_cast<int>(ig + (255 - ig) * boost)));
								ib = static_cast<uint8_t>(std::min(255, static_cast<int>(ib + (255 - ib) * boost)));
							}
						}
					}
					item_drawer->BlitItem(sprite_batch, primitive_renderer, sprite_drawer, creature_drawer, draw_x, draw_y, tile, item, options, false, ir, ig, ib);
				}
			}
			// monster/npc on tile
			if (tile->creature && options.show_creatures) {
				creature_drawer->BlitCreature(sprite_batch, sprite_drawer, draw_x, draw_y, tile->creature);
				if (creature_name_drawer) {
					creature_name_drawer->addLabel(location->getPosition(), tile->creature->getName(), tile->creature);
				}
			}
		}

		if (view.zoom < 10.0) {
			// markers (waypoint, house exit, town temple, spawn)
			marker_drawer->draw(sprite_batch, sprite_drawer, draw_x, draw_y, tile, waypoint, current_house_id, *editor, options);
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
		for (const auto* item : tile->items) {
			if (item->hasLight()) {
				light_buffer.AddLight(position.x, position.y, position.z, item->getLight());
			}
		}
	}
}
