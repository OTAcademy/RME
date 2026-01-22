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
#include "rendering/render_view.h"

class GridDrawer;

class MapCanvas;
class LightDrawer;

class MapDrawer {
	MapCanvas* canvas;
	Editor& editor;
	DrawingOptions options;
	RenderView view;
	std::shared_ptr<LightDrawer> light_drawer;
	std::unique_ptr<TooltipDrawer> tooltip_drawer;
	std::unique_ptr<GridDrawer> grid_drawer;

	uint32_t current_house_id;

	// Texture bind caching for performance
	GLuint last_bound_texture_ = 0;

protected:
	std::ostringstream tooltip;

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
	void DrawDraggingShadow();
	void DrawHigherFloors();
	void DrawSelectionBox();
	void DrawLiveCursors();
	void DrawBrush();
	void DrawIngameBox();
	void DrawGrid();
	void DrawTooltips();

	void DrawLight();

	void TakeScreenshot(uint8_t* screenshot_buffer);

	DrawingOptions& getOptions() {
		return options;
	}

protected:
	void BlitItem(int& screenx, int& screeny, const Tile* tile, Item* item, bool ephemeral = false, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitItem(int& screenx, int& screeny, const Position& pos, Item* item, bool ephemeral = false, int red = 255, int green = 255, int blue = 255, int alpha = 255, const Tile* tile = nullptr);
	void BlitSpriteType(int screenx, int screeny, uint32_t spriteid, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitSpriteType(int screenx, int screeny, GameSprite* spr, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitCreature(int screenx, int screeny, const Creature* c, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitCreature(int screenx, int screeny, const Outfit& outfit, Direction dir, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitSquare(int sx, int sy, int red, int green, int blue, int alpha, int size = 0);
	void DrawRawBrush(int screenx, int screeny, ItemType* itemType, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
	void DrawTile(TileLocation* tile);
	void DrawBrushIndicator(int x, int y, Brush* brush, uint8_t r, uint8_t g, uint8_t b);
	void DrawHookIndicator(int x, int y, const ItemType& type);
	void AddLight(TileLocation* location);

	enum BrushColor {
		COLOR_BRUSH,
		COLOR_HOUSE_BRUSH,
		COLOR_FLAG_BRUSH,
		COLOR_SPAWN_BRUSH,
		COLOR_ERASER,
		COLOR_VALID,
		COLOR_INVALID,
		COLOR_BLANK,
	};

	void getColor(Brush* brush, const Position& position, uint8_t& r, uint8_t& g, uint8_t& b);
	void glBlitTexture(int sx, int sy, int texture_number, int red, int green, int blue, int alpha);
	void glBlitSquare(int sx, int sy, int red, int green, int blue, int alpha, int size = 0);
	void glColor(wxColor color);
	void glColor(BrushColor color);
	void glColorCheck(Brush* brush, const Position& pos);
};

#endif
