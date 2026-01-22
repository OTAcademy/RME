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

#ifndef RME_MAP_DRAWER_H_
#define RME_MAP_DRAWER_H_
#include <memory>
class GameSprite;

class TooltipDrawer;

// Storage during drawing, for option caching
#include "rendering/drawing_options.h"
#include "definitions.h"
#include "outfit.h"
#include "creature.h"

#include "rendering/render_view.h"

class GridDrawer;

class MapCanvas;
class LightDrawer;
class LiveCursorDrawer;
class SelectionDrawer;
class BrushCursorDrawer;
class BrushOverlayDrawer;
class DragShadowDrawer;
class FloorDrawer;
class SpriteDrawer;
class ItemDrawer;
class CreatureDrawer;

class MapDrawer {
	MapCanvas* canvas;
	Editor& editor;
	DrawingOptions options;
	RenderView view;
	std::shared_ptr<LightDrawer> light_drawer;
	std::unique_ptr<TooltipDrawer> tooltip_drawer;
	std::unique_ptr<GridDrawer> grid_drawer;
	std::unique_ptr<LiveCursorDrawer> live_cursor_drawer;
	std::unique_ptr<SelectionDrawer> selection_drawer;
	std::unique_ptr<BrushCursorDrawer> brush_cursor_drawer;
	std::unique_ptr<BrushOverlayDrawer> brush_overlay_drawer;
	std::unique_ptr<DragShadowDrawer> drag_shadow_drawer;
	std::unique_ptr<FloorDrawer> floor_drawer;
	std::unique_ptr<SpriteDrawer> sprite_drawer;
	std::unique_ptr<CreatureDrawer> creature_drawer;
	std::unique_ptr<ItemDrawer> item_drawer;

	uint32_t current_house_id;

protected:
	std::ostringstream tooltip;

	friend class BrushOverlayDrawer;
	friend class DragShadowDrawer;
	friend class FloorDrawer;

public:
	MapDrawer(MapCanvas* canvas);
	~MapDrawer();

	bool dragging;
	bool dragging_draw;

	void SetupVars();
	void SetupGL();
	void Release();

	void Draw();
	void DrawBackground();
	void DrawMap();
	void DrawLiveCursors();
	void DrawIngameBox();

	void DrawGrid();
	void DrawTooltips();

	void DrawLight();

	void TakeScreenshot(uint8_t* screenshot_buffer);

	DrawingOptions& getOptions() {
		return options;
	}

protected:
	void DrawTile(TileLocation* tile);
	void AddLight(TileLocation* location);
};

#endif
