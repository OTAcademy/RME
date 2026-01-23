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
#include "rendering/ui/map_display.h"
#include "copybuffer.h"
#include "live_socket.h"
#include "rendering/core/graphics.h"

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
#include "rendering/utilities/light_drawer.h"
#include "rendering/ui/tooltip_drawer.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/render_view.h"

#include "rendering/drawers/overlays/grid_drawer.h"
#include "rendering/drawers/cursors/live_cursor_drawer.h"
#include "rendering/drawers/overlays/selection_drawer.h"
#include "rendering/drawers/cursors/brush_cursor_drawer.h"
#include "rendering/drawers/overlays/brush_overlay_drawer.h"
#include "rendering/drawers/cursors/drag_shadow_drawer.h"
#include "rendering/drawers/tiles/floor_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/drawers/entities/item_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/overlays/marker_drawer.h"
#include "rendering/drawers/overlays/preview_drawer.h"
#include "rendering/drawers/tiles/shade_drawer.h"
#include "rendering/drawers/tiles/tile_color_calculator.h"
#include "rendering/io/screen_capture.h"
#include "rendering/drawers/tiles/tile_renderer.h"

MapDrawer::MapDrawer(MapCanvas* canvas) :
	canvas(canvas), editor(canvas->editor) {
	light_drawer = std::make_shared<LightDrawer>();
	tooltip_drawer = std::make_unique<TooltipDrawer>();

	sprite_drawer = std::make_unique<SpriteDrawer>();
	creature_drawer = std::make_unique<CreatureDrawer>();
	floor_drawer = std::make_unique<FloorDrawer>();
	item_drawer = std::make_unique<ItemDrawer>();
	marker_drawer = std::make_unique<MarkerDrawer>();

	tile_renderer = std::make_unique<TileRenderer>(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), floor_drawer.get(), marker_drawer.get(), tooltip_drawer.get(), &editor);

	grid_drawer = std::make_unique<GridDrawer>();
	live_cursor_drawer = std::make_unique<LiveCursorDrawer>();
	selection_drawer = std::make_unique<SelectionDrawer>();
	brush_cursor_drawer = std::make_unique<BrushCursorDrawer>();
	brush_overlay_drawer = std::make_unique<BrushOverlayDrawer>();
	drag_shadow_drawer = std::make_unique<DragShadowDrawer>();
	preview_drawer = std::make_unique<PreviewDrawer>();
	shade_drawer = std::make_unique<ShadeDrawer>();
}

MapDrawer::~MapDrawer() {
	Release();
}

void MapDrawer::SetupVars() {
	view.Setup(canvas, options);
}

void MapDrawer::SetupGL() {
	// Reset texture cache at the start of each frame
	sprite_drawer->ResetCache();

	view.SetupGL();
}

void MapDrawer::Release() {
	tooltip_drawer->clear(); // Note: tooltip_drawer uses clear(), distinct from LightDrawer

	if (light_drawer) {
		light_drawer->unloadGLTexture();
	}
}

void MapDrawer::Draw() {
	light_buffer.Clear();

	DrawBackground();
	DrawMap();
	if (options.isDrawLight()) {
		DrawLight();
	}
	drag_shadow_drawer->draw(this, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options);
	floor_drawer->draw(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor); // Preserving logic
	floor_drawer->draw(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor); // Preserving double call from original code? Verified in user code.

	if (options.boundbox_selection) {
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
	view.Clear();
}

void MapDrawer::DrawMap() {
	bool live_client = editor.IsLiveClient();

	bool only_colors = options.show_as_minimap || options.show_only_colors;

	// Enable texture mode
	if (!only_colors) {
		glEnable(GL_TEXTURE_2D);
	}

	for (int map_z = view.start_z; map_z >= view.superend_z; map_z--) {
		if (map_z == view.end_z && view.start_z != view.end_z) {
			shade_drawer->draw(view, options);
		}

		if (map_z >= view.end_z) {
			DrawMapLayer(map_z, live_client);
		}

		if (only_colors) {
			glEnable(GL_TEXTURE_2D);
		}

		preview_drawer->draw(canvas, view, map_z, options, editor, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), options.current_house_id);

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

void MapDrawer::DrawTooltips() {
	// Origin calls draw(zoom, TileSize) or similar, but TooltipDrawer logic might vary.
	// The user file had: tooltip_drawer->draw(view.zoom, TileSize);
	// We'll keep it as user had it to avoid breaking tooltip logic.
	tooltip_drawer->draw(view.zoom, TileSize);
}

void MapDrawer::DrawMapLayer(int map_z, bool live_client) {
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
						tile_renderer->DrawTile(location, view, options, options.current_house_id, tooltip);
						// draw light, but only if not zoomed too far
						if (location && options.isDrawLight() && view.zoom <= 10.0) {
							tile_renderer->AddLight(location, view, options, light_buffer);
						}
					}
				}
			} else {
				if (!nd->isRequested(map_z > GROUND_LAYER)) {
					// Request the node
					editor.QueryNode(nd_map_x, nd_map_y, map_z > GROUND_LAYER);
					nd->setRequested(map_z > GROUND_LAYER, true);
				}
				grid_drawer->DrawNodeLoadingPlaceholder(nd_map_x, nd_map_y, view);
			}
		}
	}
}

void MapDrawer::DrawLight() {
	light_drawer->draw(view.start_x, view.start_y, view.end_x, view.end_y, view.view_scroll_x, view.view_scroll_y, options.experimental_fog, light_buffer);
}

void MapDrawer::TakeScreenshot(uint8_t* screenshot_buffer) {
	ScreenCapture::Capture(view.screensize_x, view.screensize_y, screenshot_buffer);
}
