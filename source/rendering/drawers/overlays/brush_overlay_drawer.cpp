//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/overlays/brush_overlay_drawer.h"
#include "rendering/map_drawer.h"
#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "rendering/drawers/cursors/brush_cursor_drawer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "rendering/core/graphics.h"
#include "editor.h"
#include "gui.h"
#include "sprites.h"
#include "settings.h"
#include "outfit.h"
#include "sprites.h"
#include "settings.h"
#include "outfit.h"
#include "definitions.h"
#include "creatures.h"
#include "settings.h"
#include "definitions.h"

#include "brush.h"

#include "rendering/ui/drawing_controller.h"
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

#include "waypoint_brush.h"

// Helper to get color from config
glm::vec4 BrushOverlayDrawer::get_brush_color(BrushColor color) {
	glm::vec4 c(1.0f);
	switch (color) {
		case COLOR_BRUSH:
			c = glm::vec4(
				g_settings.getInteger(Config::CURSOR_RED) / 255.0f,
				g_settings.getInteger(Config::CURSOR_GREEN) / 255.0f,
				g_settings.getInteger(Config::CURSOR_BLUE) / 255.0f,
				g_settings.getInteger(Config::CURSOR_ALPHA) / 255.0f
			);
			break;

		case COLOR_FLAG_BRUSH:
		case COLOR_HOUSE_BRUSH:
			c = glm::vec4(
				g_settings.getInteger(Config::CURSOR_ALT_RED) / 255.0f,
				g_settings.getInteger(Config::CURSOR_ALT_GREEN) / 255.0f,
				g_settings.getInteger(Config::CURSOR_ALT_BLUE) / 255.0f,
				g_settings.getInteger(Config::CURSOR_ALT_ALPHA) / 255.0f
			);
			break;

		case COLOR_SPAWN_BRUSH:
		case COLOR_ERASER:
		case COLOR_INVALID:
			c = glm::vec4(166.0f / 255.0f, 0.0f, 0.0f, 128.0f / 255.0f);
			break;

		case COLOR_VALID:
			c = glm::vec4(0.0f, 166.0f / 255.0f, 0.0f, 128.0f / 255.0f);
			break;

		default:
			c = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
			break;
	}
	return c;
}

glm::vec4 BrushOverlayDrawer::get_check_color(Brush* brush, Editor& editor, const Position& pos) {
	if (brush->canDraw(&editor.map, pos)) {
		return get_brush_color(COLOR_VALID);
	} else {
		return get_brush_color(COLOR_INVALID);
	}
}

BrushOverlayDrawer::BrushOverlayDrawer() {
}

BrushOverlayDrawer::~BrushOverlayDrawer() {
}

void BrushOverlayDrawer::draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, MapDrawer* drawer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options, Editor& editor) {
	if (!g_gui.IsDrawingMode()) {
		return;
	}
	if (!g_gui.GetCurrentBrush()) {
		return;
	}
	if (options.ingame) {
		return;
	}

	Brush* brush = g_gui.GetCurrentBrush();

	BrushColor brushColorType = COLOR_BLANK;
	if (brush->isTerrain() || brush->isTable() || brush->isCarpet()) {
		brushColorType = COLOR_BRUSH;
	} else if (brush->isHouse()) {
		brushColorType = COLOR_HOUSE_BRUSH;
	} else if (brush->isFlag()) {
		brushColorType = COLOR_FLAG_BRUSH;
	} else if (brush->isSpawn()) {
		brushColorType = COLOR_SPAWN_BRUSH;
	} else if (brush->isEraser()) {
		brushColorType = COLOR_ERASER;
	}

	glm::vec4 brushColor = get_brush_color(brushColorType);

	if (drawer->canvas->drawing_controller->IsDraggingDraw()) {
		ASSERT(brush->canDrag());

		if (brush->isWall()) {
			int last_click_start_map_x = std::min(drawer->canvas->last_click_map_x, view.mouse_map_x);
			int last_click_start_map_y = std::min(drawer->canvas->last_click_map_y, view.mouse_map_y);
			int last_click_end_map_x = std::max(drawer->canvas->last_click_map_x, view.mouse_map_x) + 1;
			int last_click_end_map_y = std::max(drawer->canvas->last_click_map_y, view.mouse_map_y) + 1;

			int last_click_start_sx = last_click_start_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			int last_click_start_sy = last_click_start_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();
			int last_click_end_sx = last_click_end_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			int last_click_end_sy = last_click_end_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();

			int delta_x = last_click_end_sx - last_click_start_sx;
			int delta_y = last_click_end_sy - last_click_start_sy;

			if (g_gui.gfx.ensureAtlasManager()) {
				const AtlasManager& atlas = *g_gui.gfx.getAtlasManager();
				// Top
				sprite_batch.drawRect((float)last_click_start_sx, (float)last_click_start_sy, (float)(last_click_end_sx - last_click_start_sx), (float)TileSize, brushColor, atlas);

				// Bottom
				if (delta_y > TileSize) {
					sprite_batch.drawRect((float)last_click_start_sx, (float)(last_click_end_sy - TileSize), (float)(last_click_end_sx - last_click_start_sx), (float)TileSize, brushColor, atlas);
				}

				// Right
				if (delta_x > TileSize && delta_y > TileSize) {
					sprite_batch.drawRect((float)(last_click_end_sx - TileSize), (float)(last_click_start_sy + TileSize), (float)TileSize, (float)(last_click_end_sy - last_click_start_sy - 2 * TileSize + TileSize), brushColor, atlas);
					float h = (last_click_end_sy - TileSize) - (last_click_start_sy + TileSize);
					sprite_batch.drawRect((float)(last_click_end_sx - TileSize), (float)(last_click_start_sy + TileSize), (float)TileSize, h, brushColor, atlas);
				}

				// Left
				if (delta_y > TileSize) {
					float h = (last_click_end_sy - TileSize) - (last_click_start_sy + TileSize);
					sprite_batch.drawRect((float)last_click_start_sx, (float)(last_click_start_sy + TileSize), (float)TileSize, h, brushColor, atlas);
				}
			}
		} else {
			// if (brush->isRaw()) { glEnable(GL_TEXTURE_2D); } -> handled by DrawRawBrush or BatchRenderer

			if (g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE || brush->isSpawn()) {
				if (brush->isRaw() || brush->isOptionalBorder()) {
					int start_x, end_x;
					int start_y, end_y;

					if (view.mouse_map_x < drawer->canvas->last_click_map_x) {
						start_x = view.mouse_map_x;
						end_x = drawer->canvas->last_click_map_x;
					} else {
						start_x = drawer->canvas->last_click_map_x;
						end_x = view.mouse_map_x;
					}
					if (view.mouse_map_y < drawer->canvas->last_click_map_y) {
						start_y = view.mouse_map_y;
						end_y = drawer->canvas->last_click_map_y;
					} else {
						start_y = drawer->canvas->last_click_map_y;
						end_y = view.mouse_map_y;
					}

					RAWBrush* raw_brush = nullptr;
					if (brush->isRaw()) {
						raw_brush = brush->asRaw();
					}

					for (int y = start_y; y <= end_y; y++) {
						int cy = y * TileSize - view.view_scroll_y - view.getFloorAdjustment();
						for (int x = start_x; x <= end_x; x++) {
							int cx = x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
							if (brush->isOptionalBorder()) {
								if (g_gui.gfx.ensureAtlasManager()) {
									const AtlasManager& atlas = *g_gui.gfx.getAtlasManager();
									sprite_batch.drawRect((float)cx, (float)cy, (float)TileSize, (float)TileSize, get_check_color(brush, editor, Position(x, y, view.floor)), atlas);
								}
							} else {
								item_drawer->DrawRawBrush(sprite_batch, sprite_drawer, cx, cy, raw_brush->getItemType(), 160, 160, 160, 160);
							}
						}
					}
				} else {
					int last_click_start_map_x = std::min(drawer->canvas->last_click_map_x, view.mouse_map_x);
					int last_click_start_map_y = std::min(drawer->canvas->last_click_map_y, view.mouse_map_y);
					int last_click_end_map_x = std::max(drawer->canvas->last_click_map_x, view.mouse_map_x) + 1;
					int last_click_end_map_y = std::max(drawer->canvas->last_click_map_y, view.mouse_map_y) + 1;

					int last_click_start_sx = last_click_start_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
					int last_click_start_sy = last_click_start_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();
					int last_click_end_sx = last_click_end_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
					int last_click_end_sy = last_click_end_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();

					float w = last_click_end_sx - last_click_start_sx;
					float h = last_click_end_sy - last_click_start_sy;
					if (g_gui.gfx.ensureAtlasManager()) {
						sprite_batch.drawRect((float)last_click_start_sx, (float)last_click_start_sy, w, h, brushColor, *g_gui.gfx.getAtlasManager());
					}
				}
			} else if (g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
				// Calculate drawing offsets
				int start_x, end_x;
				int start_y, end_y;
				int width = std::max(
					std::abs(std::max(view.mouse_map_y, drawer->canvas->last_click_map_y) - std::min(view.mouse_map_y, drawer->canvas->last_click_map_y)),
					std::abs(std::max(view.mouse_map_x, drawer->canvas->last_click_map_x) - std::min(view.mouse_map_x, drawer->canvas->last_click_map_x))
				);

				if (view.mouse_map_x < drawer->canvas->last_click_map_x) {
					start_x = drawer->canvas->last_click_map_x - width;
					end_x = drawer->canvas->last_click_map_x;
				} else {
					start_x = drawer->canvas->last_click_map_x;
					end_x = drawer->canvas->last_click_map_x + width;
				}

				if (view.mouse_map_y < drawer->canvas->last_click_map_y) {
					start_y = drawer->canvas->last_click_map_y - width;
					end_y = drawer->canvas->last_click_map_y;
				} else {
					start_y = drawer->canvas->last_click_map_y;
					end_y = drawer->canvas->last_click_map_y + width;
				}

				int center_x = start_x + (end_x - start_x) / 2;
				int center_y = start_y + (end_y - start_y) / 2;
				float radii = width / 2.0f + 0.005f;

				RAWBrush* raw_brush = nullptr;
				if (brush->isRaw()) {
					raw_brush = brush->asRaw();
				}

				for (int y = start_y - 1; y <= end_y + 1; y++) {
					int cy = y * TileSize - view.view_scroll_y - view.getFloorAdjustment();
					float dy = center_y - y;
					for (int x = start_x - 1; x <= end_x + 1; x++) {
						int cx = x * TileSize - view.view_scroll_x - view.getFloorAdjustment();

						float dx = center_x - x;
						float distance = sqrt(dx * dx + dy * dy);
						if (distance < radii) {
							if (brush->isRaw()) {
								item_drawer->DrawRawBrush(sprite_batch, sprite_drawer, cx, cy, raw_brush->getItemType(), 160, 160, 160, 160);
							} else {
								if (g_gui.gfx.ensureAtlasManager()) {
									sprite_batch.drawRect((float)cx, (float)cy, (float)TileSize, (float)TileSize, brushColor, *g_gui.gfx.getAtlasManager());
								}
							}
						}
					}
				}
			}

			// if (brush->isRaw()) { glDisable(GL_TEXTURE_2D); }
		}
	} else {
		if (brush->isWall()) {
			int start_map_x = view.mouse_map_x - g_gui.GetBrushSize();
			int start_map_y = view.mouse_map_y - g_gui.GetBrushSize();
			int end_map_x = view.mouse_map_x + g_gui.GetBrushSize() + 1;
			int end_map_y = view.mouse_map_y + g_gui.GetBrushSize() + 1;

			int start_sx = start_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			int start_sy = start_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();
			int end_sx = end_map_x * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			int end_sy = end_map_y * TileSize - view.view_scroll_y - view.getFloorAdjustment();

			int delta_x = end_sx - start_sx;
			int delta_y = end_sy - start_sy;

			if (g_gui.gfx.ensureAtlasManager()) {
				const AtlasManager& atlas = *g_gui.gfx.getAtlasManager();
				// Top
				sprite_batch.drawRect((float)start_sx, (float)start_sy, (float)(end_sx - start_sx), (float)TileSize, brushColor, atlas);

				// Bottom
				if (delta_y > TileSize) {
					sprite_batch.drawRect((float)start_sx, (float)(end_sy - TileSize), (float)(end_sx - start_sx), (float)TileSize, brushColor, atlas);
				}

				// Right
				if (delta_x > TileSize && delta_y > TileSize) {
					sprite_batch.drawRect((float)(end_sx - TileSize), (float)(start_sy + TileSize), (float)TileSize, (float)(end_sy - start_sy - 2 * TileSize + TileSize), brushColor, atlas);
				}

				// Left
				if (delta_y > TileSize) {
					sprite_batch.drawRect((float)start_sx, (float)(start_sy + TileSize), (float)TileSize, (float)(end_sy - start_sy - 2 * TileSize + TileSize), brushColor, atlas);
				}
			}
		} else if (brush->isDoor()) {
			int cx = (view.mouse_map_x) * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			int cy = (view.mouse_map_y) * TileSize - view.view_scroll_y - view.getFloorAdjustment();

			if (g_gui.gfx.ensureAtlasManager()) {
				sprite_batch.drawRect((float)cx, (float)cy, (float)TileSize, (float)TileSize, get_check_color(brush, editor, Position(view.mouse_map_x, view.mouse_map_y, view.floor)), *g_gui.gfx.getAtlasManager());
			}
		} else if (brush->isCreature()) {
			// glEnable(GL_TEXTURE_2D);
			int cy = (view.mouse_map_y) * TileSize - view.view_scroll_y - view.getFloorAdjustment();
			int cx = (view.mouse_map_x) * TileSize - view.view_scroll_x - view.getFloorAdjustment();
			CreatureBrush* creature_brush = brush->asCreature();
			if (creature_brush->canDraw(&editor.map, Position(view.mouse_map_x, view.mouse_map_y, view.floor))) {
				creature_drawer->BlitCreature(sprite_batch, sprite_drawer, cx, cy, creature_brush->getType()->outfit, SOUTH, 255, 255, 255, 160);
			} else {
				creature_drawer->BlitCreature(sprite_batch, sprite_drawer, cx, cy, creature_brush->getType()->outfit, SOUTH, 255, 64, 64, 160);
			}
			// glDisable(GL_TEXTURE_2D);
		} else if (!brush->isDoodad()) {
			RAWBrush* raw_brush = nullptr;
			if (brush->isRaw()) { // Textured brush
				// glEnable(GL_TEXTURE_2D);
				raw_brush = brush->asRaw();
			}

			for (int y = -g_gui.GetBrushSize() - 1; y <= g_gui.GetBrushSize() + 1; y++) {
				int cy = (view.mouse_map_y + y) * TileSize - view.view_scroll_y - view.getFloorAdjustment();
				for (int x = -g_gui.GetBrushSize() - 1; x <= g_gui.GetBrushSize() + 1; x++) {
					int cx = (view.mouse_map_x + x) * TileSize - view.view_scroll_x - view.getFloorAdjustment();
					if (g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE) {
						if (x >= -g_gui.GetBrushSize() && x <= g_gui.GetBrushSize() && y >= -g_gui.GetBrushSize() && y <= g_gui.GetBrushSize()) {
							if (brush->isRaw()) {
								item_drawer->DrawRawBrush(sprite_batch, sprite_drawer, cx, cy, raw_brush->getItemType(), 160, 160, 160, 160);
							} else {
								if (brush->isWaypoint()) {
									uint8_t r, g, b;
									get_color(brush, editor, Position(view.mouse_map_x + x, view.mouse_map_y + y, view.floor), r, g, b);
									drawer->brush_cursor_drawer->draw(sprite_batch, primitive_renderer, cx, cy, brush, r, g, b);
								} else {
									glm::vec4 c = brushColor;
									if (brush->isHouseExit() || brush->isOptionalBorder()) {
										c = get_check_color(brush, editor, Position(view.mouse_map_x + x, view.mouse_map_y + y, view.floor));
									}
									if (g_gui.gfx.ensureAtlasManager()) {
										sprite_batch.drawRect((float)cx, (float)cy, (float)TileSize, (float)TileSize, c, *g_gui.gfx.getAtlasManager());
									}
								}
							}
						}
					} else if (g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
						double distance = sqrt(double(x * x) + double(y * y));
						if (distance < g_gui.GetBrushSize() + 0.005) {
							if (brush->isRaw()) {
								item_drawer->DrawRawBrush(sprite_batch, sprite_drawer, cx, cy, raw_brush->getItemType(), 160, 160, 160, 160);
							} else {
								if (brush->isWaypoint()) {
									uint8_t r, g, b;
									get_color(brush, editor, Position(view.mouse_map_x + x, view.mouse_map_y + y, view.floor), r, g, b);
									drawer->brush_cursor_drawer->draw(sprite_batch, primitive_renderer, cx, cy, brush, r, g, b);
								} else {
									glm::vec4 c = brushColor;
									if (brush->isHouseExit() || brush->isOptionalBorder()) {
										c = get_check_color(brush, editor, Position(view.mouse_map_x + x, view.mouse_map_y + y, view.floor));
									}
									if (g_gui.gfx.ensureAtlasManager()) {
										sprite_batch.drawRect((float)cx, (float)cy, (float)TileSize, (float)TileSize, c, *g_gui.gfx.getAtlasManager());
									}
								}
							}
						}
					}
				}
			}

			// if (brush->isRaw()) { // Textured brush
			// 	glDisable(GL_TEXTURE_2D);
			// }
		}
	}
}

void BrushOverlayDrawer::get_color(Brush* brush, Editor& editor, const Position& position, uint8_t& r, uint8_t& g, uint8_t& b) {
	if (brush->canDraw(&editor.map, position)) {
		if (brush->isWaypoint()) {
			r = 0x00;
			g = 0xff, b = 0x00;
		} else {
			r = 0x00;
			g = 0x00, b = 0xff;
		}
	} else {
		r = 0xff;
		g = 0x00, b = 0x00;
	}
}
