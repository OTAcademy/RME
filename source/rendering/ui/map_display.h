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

#ifndef RME_DISPLAY_WINDOW_H_
#define RME_DISPLAY_WINDOW_H_

#include "editor/action.h"
#include "map/tile.h"
#include "game/creature.h"
#include "rendering/utilities/frame_pacer.h"

#include "ui/map_popup_menu.h"
#include "ui/map_popup_menu.h"
#include "game/animation_timer.h"
#include "rendering/core/graphics.h"
#include <memory>

struct NVGcontext;

class Item;
class Creature;
class MapWindow;
class AnimationTimer;
class AnimationTimer;
class MapDrawer;
class SelectionController;
class DrawingController;
class ScreenshotController;
class MapMenuHandler;

class MapCanvas : public wxGLCanvas {
public:
	MapCanvas(MapWindow* parent, Editor& editor, int* attriblist);
	~MapCanvas() override;
	void Reset();

	// All events
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event) { }

	void OnMouseMove(wxMouseEvent& event);
	void OnMouseLeftRelease(wxMouseEvent& event);
	void OnMouseLeftClick(wxMouseEvent& event);
	void OnMouseLeftDoubleClick(wxMouseEvent& event);
	void OnMouseCenterClick(wxMouseEvent& event);
	void OnMouseCenterRelease(wxMouseEvent& event);
	void OnMouseRightClick(wxMouseEvent& event);
	void OnMouseRightRelease(wxMouseEvent& event);

	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnWheel(wxMouseEvent& event);
	void OnGainMouse(wxMouseEvent& event);
	void OnLoseMouse(wxMouseEvent& event);

	// Mouse events handlers (called by the above)
	void OnMouseActionRelease(wxMouseEvent& event);
	void OnMouseActionClick(wxMouseEvent& event);
	void OnMouseCameraClick(wxMouseEvent& event);
	void OnMouseCameraRelease(wxMouseEvent& event);
	void OnMousePropertiesClick(wxMouseEvent& event);
	void OnMousePropertiesRelease(wxMouseEvent& event);

	void Refresh();

	void ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y);
	void MouseToMap(int* map_x, int* map_y) {
		ScreenToMap(cursor_x, cursor_y, map_x, map_y);
	}
	void GetScreenCenter(int* map_x, int* map_y);

	void StartPasting();
	void EndPasting();
	void EnterSelectionMode();
	void EnterDrawingMode();

	void UpdatePositionStatus(int x = -1, int y = -1);
	void UpdateZoomStatus();

	void ChangeFloor(int new_floor);
	int GetFloor() const {
		return floor;
	}
	double GetZoom() const {
		return zoom;
	}
	void SetZoom(double value);
	void GetViewBox(int* view_scroll_x, int* view_scroll_y, int* screensize_x, int* screensize_y) const;

	Position GetCursorPosition() const;

	void TakeScreenshot(wxFileName path, wxString format);

	enum {
		BLOCK_SIZE = 100
	};

	inline int getFillIndex(int x, int y) const {
		return x + BLOCK_SIZE * y;
	}

	static bool processed[BLOCK_SIZE * BLOCK_SIZE];
	Editor& editor;
	std::unique_ptr<MapDrawer> drawer;
	int keyCode;

	// View related
	int floor;
	double zoom;
	int cursor_x;
	int cursor_y;

	bool dragging;
	bool boundbox_selection;
	bool screendragging;
	bool isPasting() const;

	std::unique_ptr<ScreenshotController> screenshot_controller;

	int last_cursor_map_x;
	int last_cursor_map_y;
	int last_cursor_map_z;

	int last_click_map_x;
	int last_click_map_y;
	int last_click_map_z;
	int last_click_abs_x;
	int last_click_abs_y;
	int last_click_x;
	int last_click_y;

	int last_mmb_click_x;
	int last_mmb_click_y;

	// HUD cache
	std::string hud_cached_text;
	float hud_cached_bounds[4] = { 0 };
	size_t hud_cached_selection_count = 0;
	int hud_cached_x = -1;
	int hud_cached_y = -1;
	int hud_cached_z = -1;
	double hud_cached_zoom = -1.0;

	int view_scroll_x;
	int view_scroll_y;

	uint32_t current_house_id;

	wxStopWatch refresh_watch;
	std::unique_ptr<MapPopupMenu> popup_menu;
	std::unique_ptr<AnimationTimer> animation_timer;

	FramePacer frame_pacer;

	friend class MapDrawer;
	friend class SelectionDrawer;
	friend class BrushOverlayDrawer;
	friend class DragShadowDrawer;
	friend class PreviewDrawer;
	friend class SelectionController;
	friend class DrawingController;

	std::unique_ptr<SelectionController> selection_controller;
	std::unique_ptr<DrawingController> drawing_controller;
	std::unique_ptr<MapMenuHandler> menu_handler;

private:
	MapWindow* GetMapWindow() const;
	bool renderer_initialized = false;
	std::unique_ptr<NVGcontext, NVGDeleter> m_nvg;
};

#endif
