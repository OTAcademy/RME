//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/drawers/minimap_drawer.h"
#include "rendering/ui/map_display.h"
#include "editor.h"
#include "map.h"
#include "tile.h"
#include "gui.h"

// Included for minimap_color
#include "rendering/core/graphics.h"

MinimapDrawer::MinimapDrawer() :
	last_start_x(0), last_start_y(0) {
	for (int i = 0; i < 256; ++i) {
		pens[i] = newd wxPen(wxColor(minimap_color[i].red, minimap_color[i].green, minimap_color[i].blue));
	}
}

MinimapDrawer::~MinimapDrawer() {
	for (int i = 0; i < 256; ++i) {
		delete pens[i];
	}
}

void MinimapDrawer::Draw(wxDC& pdc, const wxSize& size, Editor& editor, MapCanvas* canvas) {
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	int window_width = size.GetWidth();
	int window_height = size.GetHeight();

	int center_x, center_y;
	canvas->GetScreenCenter(&center_x, &center_y);

	int start_x, start_y;
	int end_x, end_y;
	start_x = center_x - window_width / 2;
	start_y = center_y - window_height / 2;

	end_x = center_x + window_width / 2;
	end_y = center_y + window_height / 2;

	if (start_x < 0) {
		start_x = 0;
		end_x = window_width;
	} else if (end_x > editor.map.getWidth()) {
		start_x = editor.map.getWidth() - window_width;
		end_x = editor.map.getWidth();
	}
	if (start_y < 0) {
		start_y = 0;
		end_y = window_height;
	} else if (end_y > editor.map.getHeight()) {
		start_y = editor.map.getHeight() - window_height;
		end_y = editor.map.getHeight();
	}

	start_x = std::max(start_x, 0);
	start_y = std::max(start_y, 0);
	end_x = std::min(end_x, editor.map.getWidth());
	end_y = std::min(end_y, editor.map.getHeight());

	last_start_x = start_x;
	last_start_y = start_y;

	int floor = g_gui.GetCurrentFloor();

	uint8_t last = 0;
	if (g_gui.IsRenderingEnabled()) {
		for (int y = start_y, window_y = 0; y <= end_y; ++y, ++window_y) {
			for (int x = start_x, window_x = 0; x <= end_x; ++x, ++window_x) {
				Tile* tile = editor.map.getTile(x, y, floor);
				if (tile) {
					uint8_t color = tile->getMiniMapColor();
					if (color) {
						if (last != color) {
							pdc.SetPen(*pens[color]);
							last = color;
						}
						pdc.DrawPoint(window_x, window_y);
					}
				}
			}
		}

		if (g_settings.getInteger(Config::MINIMAP_VIEW_BOX)) {
			pdc.SetPen(*wxWHITE_PEN);
			// Draw the rectangle on the minimap

			// Some view info
			int screensize_x, screensize_y;
			int view_scroll_x, view_scroll_y;

			canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

			// bounds of the view
			int view_start_x, view_start_y;
			int view_end_x, view_end_y;

			int tile_size = int(TileSize / canvas->GetZoom()); // after zoom

			int floor_offset = (floor > GROUND_LAYER ? 0 : (GROUND_LAYER - floor));

			view_start_x = view_scroll_x / TileSize + floor_offset;
			view_start_y = view_scroll_y / TileSize + floor_offset;

			view_end_x = view_start_x + screensize_x / tile_size + 1;
			view_end_y = view_start_y + screensize_y / tile_size + 1;

			for (int x = view_start_x; x <= view_end_x; ++x) {
				pdc.DrawPoint(x - start_x, view_start_y - start_y);
				pdc.DrawPoint(x - start_x, view_end_y - start_y);
			}
			for (int y = view_start_y; y < view_end_y; ++y) {
				pdc.DrawPoint(view_start_x - start_x, y - start_y);
				pdc.DrawPoint(view_end_x - start_x, y - start_y);
			}
		}
	}
}

void MinimapDrawer::ScreenToMap(int screen_x, int screen_y, int& map_x, int& map_y) {
	map_x = last_start_x + screen_x;
	map_y = last_start_y + screen_y;
}
