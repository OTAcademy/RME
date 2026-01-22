//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "editor.h"
#include "gui.h"
#include "sprites.h"
#include "rendering/map_drawer.h"
#include "rendering/map_display.h"
#include "copybuffer.h"
#include "live_socket.h"
#include "rendering/graphics.h"

#include "doodad_brush.h"
#include "creature_brush.h"
#include "house_exit_brush.h"
#include "house_brush.h"
#include "spawn_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "raw_brush.h"
#include "table_brush.h"
#include "waypoint_brush.h"
#include "rendering/light_drawer.h"
#include "rendering/tooltip_drawer.h"
#include "rendering/drawing_options.h"
#include "rendering/render_view.h"
#include "rendering/grid_drawer.h"
#include "rendering/live_cursor_drawer.h"
#include "rendering/selection_drawer.h"
#include "rendering/brush_cursor_drawer.h"
#include "rendering/brush_overlay_drawer.h"
#include "rendering/drag_shadow_drawer.h"
#include "rendering/floor_drawer.h"
#include "rendering/sprite_drawer.h"
#include "rendering/item_drawer.h"
#include "rendering/creature_drawer.h"

MapDrawer::MapDrawer(MapCanvas* canvas) :
	canvas(canvas), editor(canvas->editor) {
	light_drawer = std::make_shared<LightDrawer>();
	tooltip_drawer = std::make_unique<TooltipDrawer>();
	grid_drawer = std::make_unique<GridDrawer>();
	live_cursor_drawer = std::make_unique<LiveCursorDrawer>();
	selection_drawer = std::make_unique<SelectionDrawer>();
	brush_cursor_drawer = std::make_unique<BrushCursorDrawer>();
	brush_overlay_drawer = std::make_unique<BrushOverlayDrawer>();
	drag_shadow_drawer = std::make_unique<DragShadowDrawer>();
	floor_drawer = std::make_unique<FloorDrawer>();
	sprite_drawer = std::make_unique<SpriteDrawer>();
	creature_drawer = std::make_unique<CreatureDrawer>();
	item_drawer = std::make_unique<ItemDrawer>();
}

MapDrawer::~MapDrawer() {
	Release();
}

void MapDrawer::SetupVars() {
	view.Setup(canvas, options);
	dragging = canvas->dragging;
	dragging_draw = canvas->dragging_draw;
}

void MapDrawer::SetupGL() {
	// Reset texture cache at the start of each frame
	sprite_drawer->ResetCache();

	glViewport(0, 0, view.screensize_x, view.screensize_y);

	// Enable 2D mode
	int vPort[4];

	glGetIntegerv(GL_VIEWPORT, vPort);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, vPort[2] * view.zoom, vPort[3] * view.zoom, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.375f, 0.375f, 0.0f);
}

void MapDrawer::Release() {
	tooltip_drawer->clear();

	if (light_drawer) {
		light_drawer->clear();
	}

	// Disable 2D mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void MapDrawer::Draw() {
	DrawBackground();
	DrawMap();
	if (options.isDrawLight()) {
		DrawLight();
	}
	drag_shadow_drawer->draw(this, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options);
	floor_drawer->draw(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor);
	floor_drawer->draw(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor); // Preserving double call from original code
	if (options.dragging) {
		selection_drawer->draw(view, canvas, options);
	}
	live_cursor_drawer->draw(view, editor, options);
	brush_overlay_drawer->draw(this, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor);

	if (options.show_grid) {
		DrawGrid();
	}
	if (options.show_ingame_box) {
		DrawIngameBox();
	}
	if (options.show_tooltips) {
		DrawTooltips();
	}
}

void MapDrawer::DrawBackground() {
	// Black Background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// glAlphaFunc(GL_GEQUAL, 0.9f);
	// glEnable(GL_ALPHA_TEST);
}

void MapDrawer::DrawMap() {
	int center_x = view.start_x + int(view.screensize_x * view.zoom / 64);
	int center_y = view.start_y + int(view.screensize_y * view.zoom / 64);
	int offset_y = 2;
	int box_start_map_x = center_x - view.view_scroll_x;
	int box_start_map_y = center_y - view.view_scroll_x + offset_y;
	int box_end_map_x = center_x + ClientMapWidth;
	int box_end_map_y = center_y + ClientMapHeight + offset_y;

	bool live_client = editor.IsLiveClient();

	Brush* brush = g_gui.GetCurrentBrush();

	// The current house we're drawing
	current_house_id = 0;
	if (brush) {
		if (brush->isHouse()) {
			current_house_id = brush->asHouse()->getHouseID();
		} else if (brush->isHouseExit()) {
			current_house_id = brush->asHouseExit()->getHouseID();
		}
	}

	bool only_colors = options.show_as_minimap || options.show_only_colors;

	// Enable texture mode
	if (!only_colors) {
		glEnable(GL_TEXTURE_2D);
	}

	for (int map_z = view.start_z; map_z >= view.superend_z; map_z--) {
		if (map_z == view.end_z && view.start_z != view.end_z && options.show_shade) {
			// Draw shade
			if (!only_colors) {
				glDisable(GL_TEXTURE_2D);
			}

			glColor4ub(0, 0, 0, 128);
			glBegin(GL_QUADS);
			glVertex2f(0, int(view.screensize_y * view.zoom));
			glVertex2f(int(view.screensize_x * view.zoom), int(view.screensize_y * view.zoom));
			glVertex2f(int(view.screensize_x * view.zoom), 0);
			glVertex2f(0, 0);
			glEnd();

			if (!only_colors) {
				glEnable(GL_TEXTURE_2D);
			}
		}

		if (map_z >= view.end_z) {
			int nd_start_x = view.start_x & ~3;
			int nd_start_y = view.start_y & ~3;
			int nd_end_x = (view.end_x & ~3) + 4;
			int nd_end_y = (view.end_y & ~3) + 4;

			for (int nd_map_x = nd_start_x; nd_map_x <= nd_end_x; nd_map_x += 4) {
				for (int nd_map_y = nd_start_y; nd_map_y <= nd_end_y; nd_map_y += 4) {
					QTreeNode* nd = editor.map.getLeaf(nd_map_x, nd_map_y);
					if (!nd) {
						if (live_client) {
							nd = editor.map.createLeaf(nd_map_x, nd_map_y);
							nd->setVisible(false, false);
						} else {
							continue;
						}
					}

					if (!live_client || nd->isVisible(map_z > GROUND_LAYER)) {
						for (int map_x = 0; map_x < 4; ++map_x) {
							for (int map_y = 0; map_y < 4; ++map_y) {
								TileLocation* location = nd->getTile(map_x, map_y, map_z);
								DrawTile(location);
								// draw light, but only if not zoomed too far
								if (location && options.isDrawLight() && view.zoom <= 10.0) {
									AddLight(location);
								}
							}
						}
					} else {
						if (!nd->isRequested(map_z > GROUND_LAYER)) {
							// Request the node
							editor.QueryNode(nd_map_x, nd_map_y, map_z > GROUND_LAYER);
							nd->setRequested(map_z > GROUND_LAYER, true);
						}
						int cy = (nd_map_y)*TileSize - view.view_scroll_y - view.getFloorAdjustment();
						int cx = (nd_map_x)*TileSize - view.view_scroll_x - view.getFloorAdjustment();

						glColor4ub(255, 0, 255, 128);
						glBegin(GL_QUADS);
						glVertex2f(cx, cy + TileSize * 4);
						glVertex2f(cx + TileSize * 4, cy + TileSize * 4);
						glVertex2f(cx + TileSize * 4, cy);
						glVertex2f(cx, cy);
						glEnd();
					}
				}
			}
		}

		if (only_colors) {
			glEnable(GL_TEXTURE_2D);
		}

		// Draws the doodad preview or the paste preview (or import preview)
		if (g_gui.secondary_map != nullptr && !options.ingame) {
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
					// Position pos = topos + copypos - Position(map_x, map_y, map_z);
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
							item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, tile->ground, options, true, r, g, b, 160);
						}

						// Draw items on the tile
						if (view.zoom <= 10.0 || !options.hide_items_when_zoomed) {
							ItemVector::iterator it;
							for (it = tile->items.begin(); it != tile->items.end(); it++) {
								if ((*it)->isBorder()) {
									item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, *it, options, true, 160, r, g, b);
								} else {
									item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, *it, options, true, 160, 160, 160, 160);
								}
							}
							if (tile->creature && options.show_creatures) {
								creature_drawer->BlitCreature(sprite_drawer.get(), draw_x, draw_y, tile->creature);
							}
						}
					}
				}
			}
		}

		--view.start_x;
		--view.start_y;
		++view.end_x;
		++view.end_y;
	}

	if (!only_colors) {
		glEnable(GL_TEXTURE_2D);
	}
}

void MapDrawer::DrawIngameBox() {
	grid_drawer->DrawIngameBox(view, options);
}

void MapDrawer::DrawGrid() {
	grid_drawer->DrawGrid(view, options);
}

void MapDrawer::DrawTile(TileLocation* location) {
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
	int offset = (map_z <= GROUND_LAYER)
		? (GROUND_LAYER - map_z) * TileSize
		: TileSize * (view.floor - map_z);
	int screen_x = (map_x * TileSize) - view.view_scroll_x - offset;
	int screen_y = (map_y * TileSize) - view.view_scroll_y - offset;
	int margin = TileSize * 3; // Account for large sprites
	if (screen_x < -margin || screen_x > view.screensize_x * view.zoom + margin || screen_y < -margin || screen_y > view.screensize_y * view.zoom + margin) {
		return;
	}

	Waypoint* waypoint = canvas->editor.map.waypoints.getWaypoint(location);
	if (options.show_tooltips && location->getWaypointCount() > 0) {
		if (waypoint) {
			tooltip_drawer->writeTooltip(waypoint, tooltip);
		}
	}

	bool as_minimap = options.show_as_minimap;
	bool only_colors = as_minimap || options.show_only_colors;

	int draw_x = screen_x;
	int draw_y = screen_y;

	uint8_t r = 255, g = 255, b = 255;

	// begin filters for ground tile
	if (!as_minimap) {
		bool showspecial = options.show_only_colors || options.show_special_tiles;

		if (options.show_blocking && tile->isBlocking() && tile->size() > 0) {
			g = g / 3 * 2;
			b = b / 3 * 2;
		}

		int item_count = tile->items.size();
		if (options.highlight_items && item_count > 0 && !tile->items.back()->isBorder()) {
			static const float factor[5] = { 0.75f, 0.6f, 0.48f, 0.40f, 0.33f };
			int idx = (item_count < 5 ? item_count : 5) - 1;
			g = int(g * factor[idx]);
			r = int(r * factor[idx]);
		}

		if (options.show_spawns && location->getSpawnCount() > 0) {
			float f = 1.0f;
			for (uint32_t i = 0; i < location->getSpawnCount(); ++i) {
				f *= 0.7f;
			}
			g = uint8_t(g * f);
			b = uint8_t(b * f);
		}

		if (options.show_houses && tile->isHouseTile()) {
			if ((int)tile->getHouseID() == current_house_id) {
				r /= 2;
			} else {
				r /= 2;
				g /= 2;
			}
		} else if (showspecial && tile->isPZ()) {
			r /= 2;
			b /= 2;
		}

		if (showspecial && tile->getMapFlags() & TILESTATE_PVPZONE) {
			g = r / 4;
			b = b / 3 * 2;
		}

		if (showspecial && tile->getMapFlags() & TILESTATE_NOLOGOUT) {
			b /= 2;
		}

		if (showspecial && tile->getMapFlags() & TILESTATE_NOPVP) {
			g /= 2;
		}
	}

	if (only_colors) {
		if (as_minimap) {
			uint8_t color = tile->getMiniMapColor();
			r = (uint8_t)(int(color / 36) % 6 * 51);
			g = (uint8_t)(int(color / 6) % 6 * 51);
			b = (uint8_t)(color % 6 * 51);
			sprite_drawer->glBlitSquare(draw_x, draw_y, r, g, b, 255);
		} else if (r != 255 || g != 255 || b != 255) {
			sprite_drawer->glBlitSquare(draw_x, draw_y, r, g, b, 128);
		}
	} else {
		if (tile->ground) {
			if (options.show_preview && view.zoom <= 2.0) {
				tile->ground->animate();
			}

			item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, tile->ground, options, false, r, g, b);
		} else if (options.always_show_zones && (r != 255 || g != 255 || b != 255)) {
			item_drawer->DrawRawBrush(sprite_drawer.get(), draw_x, draw_y, &g_items[SPRITE_ZONE], r, g, b, 60);
		}
	}

	if (options.show_tooltips && map_z == view.floor && tile->ground) {
		tooltip_drawer->writeTooltip(tile->ground, tooltip, false);
	}

	// end filters for ground tile

	if (!only_colors) {
		if (view.zoom < 10.0 || !options.hide_items_when_zoomed) {
			// items on tile
			for (ItemVector::iterator it = tile->items.begin(); it != tile->items.end(); it++) {
				// item tooltip
				if (options.show_tooltips && map_z == view.floor) {
					tooltip_drawer->writeTooltip(*it, tooltip, tile->isHouseTile());
				}

				// item animation
				if (options.show_preview && view.zoom <= 2.0) {
					(*it)->animate();
				}

				// item sprite
				if ((*it)->isBorder()) {
					item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, *it, options, false, r, g, b);
				} else {
					r = 255, g = 255, b = 255;

					if (options.extended_house_shader && options.show_houses && tile->isHouseTile()) {
						if ((int)tile->getHouseID() == current_house_id) {
							r /= 2;
						} else {
							r /= 2;
							g /= 2;
						}
					}
					item_drawer->BlitItem(sprite_drawer.get(), creature_drawer.get(), draw_x, draw_y, tile, *it, options, false, r, g, b);
				}
			}
			// monster/npc on tile
			if (tile->creature && options.show_creatures) {
				creature_drawer->BlitCreature(sprite_drawer.get(), draw_x, draw_y, tile->creature);
			}
		}

		if (view.zoom < 10.0) {
			// waypoint (blue flame)
			if (!options.ingame && waypoint && options.show_waypoints) {
				sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_WAYPOINT, 64, 64, 255);
			}

			// house exit (blue splash)
			if (tile->isHouseExit() && options.show_houses) {
				if (tile->hasHouseExit(current_house_id)) {
					sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_HOUSE_EXIT, 64, 255, 255);
				} else {
					sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_HOUSE_EXIT, 64, 64, 255);
				}
			}

			// town temple (gray flag)
			if (options.show_towns && tile->isTownExit(editor.map)) {
				sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_TOWN_TEMPLE, 255, 255, 64, 170);
			}

			// spawn (purple flame)
			if (tile->spawn && options.show_spawns) {
				if (tile->spawn->isSelected()) {
					sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_SPAWN, 128, 128, 128);
				} else {
					sprite_drawer->BlitSprite(draw_x, draw_y, SPRITE_SPAWN, 255, 255, 255);
				}
			}

			// tooltips
			if (options.show_tooltips) {
				if (location->getWaypointCount() > 0) {
					tooltip_drawer->addTooltip(draw_x, draw_y, tooltip.str(), 0, 255, 0);
				} else {
					tooltip_drawer->addTooltip(draw_x, draw_y, tooltip.str());
				}
			}

			tooltip.str("");
		}
	}
}

void MapDrawer::DrawTooltips() {
	tooltip_drawer->draw(view.zoom, TileSize);
}

void MapDrawer::DrawLight() {
	// draw in-game light
	light_drawer->draw(view.start_x, view.start_y, view.end_x, view.end_y, view.view_scroll_x, view.view_scroll_y, options.experimental_fog);
}

void MapDrawer::AddLight(TileLocation* location) {
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
			light_drawer->addLight(position.x, position.y, position.z, tile->ground->getLight());
		}
	}

	bool hidden = options.hide_items_when_zoomed && view.zoom > 10.f;
	if (!hidden && !tile->items.empty()) {
		for (auto item : tile->items) {
			if (item->hasLight()) {
				light_drawer->addLight(position.x, position.y, position.z, item->getLight());
			}
		}
	}
}

void MapDrawer::TakeScreenshot(uint8_t* screenshot_buffer) {
	glFinish(); // Wait for the operation to finish

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // 1 byte alignment

	for (int i = 0; i < view.screensize_y; ++i) {
		glReadPixels(0, view.screensize_y - i, view.screensize_x, 1, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)(screenshot_buffer) + 3 * view.screensize_x * i);
	}
}
