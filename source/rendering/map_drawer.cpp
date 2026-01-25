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

#include "app/main.h"

#include "editor/editor.h"
#include "ui/gui.h"
#include "game/sprites.h"
#include <spdlog/spdlog.h>
#include "rendering/map_drawer.h"
#include "brushes/brush.h"
#include "rendering/drawers/map_layer_drawer.h"
#include "rendering/ui/map_display.h"
#include "editor/copybuffer.h"
#include "live/live_socket.h"
#include "rendering/core/graphics.h"

#include "brushes/doodad_brush.h"
#include "brushes/creature_brush.h"
#include "brushes/house_exit_brush.h"
#include "brushes/house_brush.h"
#include "brushes/spawn_brush.h"
#include "brushes/wall_brush.h"
#include "brushes/carpet_brush.h"
#include "brushes/raw_brush.h"
#include "brushes/table_brush.h"
#include "brushes/waypoint_brush.h"
#include "rendering/utilities/light_drawer.h"
#include "rendering/ui/tooltip_drawer.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/render_view.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"

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
	map_layer_drawer = std::make_unique<MapLayerDrawer>(tile_renderer.get(), grid_drawer.get(), &editor); // Initialized map_layer_drawer
	live_cursor_drawer = std::make_unique<LiveCursorDrawer>();
	selection_drawer = std::make_unique<SelectionDrawer>();
	brush_cursor_drawer = std::make_unique<BrushCursorDrawer>();
	brush_overlay_drawer = std::make_unique<BrushOverlayDrawer>();
	drag_shadow_drawer = std::make_unique<DragShadowDrawer>();
	preview_drawer = std::make_unique<PreviewDrawer>();

	shade_drawer = std::make_unique<ShadeDrawer>();

	sprite_batch = std::make_unique<SpriteBatch>();
	primitive_renderer = std::make_unique<PrimitiveRenderer>();
}

MapDrawer::~MapDrawer() {
	Release();
}

void MapDrawer::SetupVars() {
	options.current_house_id = 0;
	Brush* brush = g_gui.GetCurrentBrush();
	if (brush) {
		if (brush->isHouse()) {
			options.current_house_id = brush->asHouse()->getHouseID();
		} else if (brush->isHouseExit()) {
			options.current_house_id = brush->asHouseExit()->getHouseID();
		}
	}

	view.Setup(canvas, options);
}

void MapDrawer::SetupGL() {
	// Reset texture cache at the start of each frame
	sprite_drawer->ResetCache();

	view.SetupGL();

	view.SetupGL();

	// Ensure renderers are initialized
	if (!renderers_initialized) {
		spdlog::info("MapDrawer: Initializing renderers for canvas {:p}", (void*)canvas);
		sprite_batch->initialize();
		primitive_renderer->initialize();
		renderers_initialized = true;
	}
}

void MapDrawer::Release() {
	// tooltip_drawer->clear(); // Moved to ClearTooltips(), called explicitly after UI draw

	if (light_drawer) {
		light_drawer->unloadGLTexture();
	}
}

void MapDrawer::Draw() {
	light_buffer.Clear();

	// Begin Batches
	sprite_batch->begin(view.projectionMatrix);
	primitive_renderer->setProjectionMatrix(view.projectionMatrix);

	DrawBackground(); // Clear screen
	DrawMap();

	// Flush Map for Light Pass
	if (g_gui.gfx.ensureAtlasManager()) {
		sprite_batch->end(*g_gui.gfx.getAtlasManager());
	}
	primitive_renderer->flush();

	if (options.isDrawLight()) {
		DrawLight();
	}

	// Resume Batch for Overlays
	sprite_batch->begin(view.projectionMatrix);

	if (drag_shadow_drawer) {
		drag_shadow_drawer->draw(*sprite_batch, *primitive_renderer, this, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options);
	}
	// FloorDrawer is commented out in original?
	// floor_drawer->draw(item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor);

	if (options.boundbox_selection) {
		selection_drawer->draw(*sprite_batch, view, canvas, options);
	}
	live_cursor_drawer->draw(*sprite_batch, view, editor, options);
	brush_overlay_drawer->draw(*sprite_batch, *primitive_renderer, this, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), view, options, editor);

	if (options.show_grid) {
		DrawGrid();
	}
	if (options.show_ingame_box) {
		DrawIngameBox();
	}

	// End Batches and Flush
	if (g_gui.gfx.ensureAtlasManager()) {
		sprite_batch->end(*g_gui.gfx.getAtlasManager());
	}
	primitive_renderer->flush();

	// Tooltips are now drawn in MapCanvas::OnPaint (UI Pass)
}

void MapDrawer::DrawBackground() {
	view.Clear();
}

void MapDrawer::DrawMap() {
	bool live_client = editor.live_manager.IsClient();

	bool only_colors = options.show_as_minimap || options.show_only_colors;

	// Enable texture mode
	// if (!only_colors) {
	// 	glEnable(GL_TEXTURE_2D);
	// }

	for (int map_z = view.start_z; map_z >= view.superend_z; map_z--) {
		if (map_z == view.end_z && view.start_z != view.end_z) {
			shade_drawer->draw(*sprite_batch, view, options);
		}

		if (map_z >= view.end_z) {
			DrawMapLayer(map_z, live_client);
		}

		// if (only_colors) {
		// 	glEnable(GL_TEXTURE_2D);
		// }

		preview_drawer->draw(*sprite_batch, *primitive_renderer, canvas, view, map_z, options, editor, item_drawer.get(), sprite_drawer.get(), creature_drawer.get(), options.current_house_id);

		--view.start_x;
		--view.start_y;
		++view.end_x;
		++view.end_y;
	}

	// if (!only_colors) {
	// 	glEnable(GL_TEXTURE_2D);
	// }
}

void MapDrawer::DrawIngameBox() {
	grid_drawer->DrawIngameBox(*sprite_batch, view, options);
}

void MapDrawer::DrawGrid() {
	grid_drawer->DrawGrid(*sprite_batch, view, options);
}

void MapDrawer::DrawTooltips() {
	tooltip_drawer->draw(view);
}

void MapDrawer::DrawMapLayer(int map_z, bool live_client) {
	map_layer_drawer->Draw(*sprite_batch, *primitive_renderer, map_z, live_client, view, options, light_buffer, tooltip);
}

void MapDrawer::DrawLight() {
	light_drawer->draw(view, options.experimental_fog, light_buffer, options.global_light_color, options.light_intensity, options.ambient_light_level);
}

void MapDrawer::TakeScreenshot(uint8_t* screenshot_buffer) {
	ScreenCapture::Capture(view.screensize_x, view.screensize_y, screenshot_buffer);
}

void MapDrawer::ClearTooltips() {
	tooltip_drawer->clear();
}
