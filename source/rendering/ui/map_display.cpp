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

#include <sstream>
#include <time.h>
#include <thread>
#include <chrono>
#include <wx/wfstream.h>

#include "ui/gui.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "brushes/brush.h"
#include "game/sprites.h"
#include "map/map.h"
#include "map/tile.h"
#include "ui/old_properties_window.h"
#include "ui/properties_window.h"
#include "ui/tileset_window.h"
#include "palette/palette_window.h"
#include "palette/palette_window.h"
#include "rendering/ui/screenshot_controller.h"
#include "rendering/utilities/tile_describer.h"
#include "rendering/core/coordinate_mapper.h"
#include "rendering/ui/map_display.h"
#include "rendering/ui/map_status_updater.h"
#include "rendering/map_drawer.h"
#include "rendering/core/text_renderer.h"
#include "app/application.h"
#include "live/live_server.h"
#include "live/live_client.h"
#include "ui/browse_tile_window.h"
#include "ui/dialog_helper.h"
#include "game/animation_timer.h"
#include "ui/map_popup_menu.h"
#include "brushes/brush_utility.h"
#include "rendering/ui/clipboard_handler.h"
#include "rendering/ui/keyboard_handler.h"
#include "rendering/ui/brush_selector.h"
#include "rendering/ui/popup_action_handler.h"
#include "rendering/ui/zoom_controller.h"
#include "rendering/ui/zoom_controller.h"
#include "rendering/ui/navigation_controller.h"
#include "rendering/ui/navigation_controller.h"
#include "rendering/ui/selection_controller.h"
#include "rendering/ui/drawing_controller.h"
#include "rendering/ui/drawing_controller.h"
#include "rendering/ui/map_menu_handler.h"

#include "brushes/doodad_brush.h"
#include "brushes/house_exit_brush.h"
#include "brushes/house_brush.h"
#include "brushes/wall_brush.h"
#include "brushes/spawn_brush.h"
#include "brushes/creature_brush.h"
#include "brushes/ground_brush.h"
#include "brushes/waypoint_brush.h"
#include "brushes/raw_brush.h"
#include "brushes/carpet_brush.h"
#include "brushes/table_brush.h"

BEGIN_EVENT_TABLE(MapCanvas, wxGLCanvas)
EVT_KEY_DOWN(MapCanvas::OnKeyDown)
EVT_KEY_DOWN(MapCanvas::OnKeyUp)

// Mouse events
EVT_MOTION(MapCanvas::OnMouseMove)
EVT_LEFT_UP(MapCanvas::OnMouseLeftRelease)
EVT_LEFT_DOWN(MapCanvas::OnMouseLeftClick)
EVT_LEFT_DCLICK(MapCanvas::OnMouseLeftDoubleClick)
EVT_MIDDLE_DOWN(MapCanvas::OnMouseCenterClick)
EVT_MIDDLE_UP(MapCanvas::OnMouseCenterRelease)
EVT_RIGHT_DOWN(MapCanvas::OnMouseRightClick)
EVT_RIGHT_UP(MapCanvas::OnMouseRightRelease)
EVT_MOUSEWHEEL(MapCanvas::OnWheel)
EVT_ENTER_WINDOW(MapCanvas::OnGainMouse)
EVT_LEAVE_WINDOW(MapCanvas::OnLoseMouse)

// Drawing events
EVT_PAINT(MapCanvas::OnPaint)
EVT_ERASE_BACKGROUND(MapCanvas::OnEraseBackground)

// Menu events
// Menu events
//----
// ----
// ----
// ----
// ----
END_EVENT_TABLE()

bool MapCanvas::processed[] = { 0 };

// Helper to create attributes
static wxGLAttributes& GetCoreProfileAttributes() {
	static wxGLAttributes vAttrs = []() {
		wxGLAttributes a;
		a.PlatformDefaults().Defaults().RGBA().DoubleBuffer().Depth(24).Stencil(8).EndList();
		return a;
	}();
	return vAttrs;
}

MapCanvas::MapCanvas(MapWindow* parent, Editor& editor, int* attriblist) :
	wxGLCanvas(parent, GetCoreProfileAttributes(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
	editor(editor),
	floor(GROUND_LAYER),
	zoom(1.0),
	renderer_initialized(false),
	cursor_x(-1),
	cursor_y(-1),
	dragging(false),
	boundbox_selection(false),
	screendragging(false),

	last_cursor_map_x(-1),
	last_cursor_map_y(-1),
	last_cursor_map_z(-1),

	last_click_map_x(-1),
	last_click_map_y(-1),
	last_click_map_z(-1),
	last_click_abs_x(-1),
	last_click_abs_y(-1),
	last_click_x(-1),
	last_click_y(-1),

	last_mmb_click_x(-1),
	last_mmb_click_y(-1) {
	popup_menu = newd MapPopupMenu(editor);
	animation_timer = newd AnimationTimer(this);
	drawer = new MapDrawer(this);
	selection_controller = std::make_unique<SelectionController>(this, editor);
	drawing_controller = std::make_unique<DrawingController>(this, editor);
	screenshot_controller = std::make_unique<ScreenshotController>(this);
	menu_handler = std::make_unique<MapMenuHandler>(this, editor);
	menu_handler->BindEvents();
	keyCode = WXK_NONE;
}

MapCanvas::~MapCanvas() {
	delete popup_menu;
	delete animation_timer;

	delete drawer;
}

void MapCanvas::Refresh() {
	if (refresh_watch.Time() > g_settings.getInteger(Config::HARD_REFRESH_RATE)) {
		refresh_watch.Start();
		wxGLCanvas::Update();
	}
	wxGLCanvas::Refresh();
}

void MapCanvas::SetZoom(double value) {
	ZoomController::SetZoom(this, value);
}

void MapCanvas::GetViewBox(int* view_scroll_x, int* view_scroll_y, int* screensize_x, int* screensize_y) const {
	GetMapWindow()->GetViewSize(screensize_x, screensize_y);
	GetMapWindow()->GetViewStart(view_scroll_x, view_scroll_y);
}

MapWindow* MapCanvas::GetMapWindow() const {
	return static_cast<MapWindow*>(GetParent());
}

void MapCanvas::OnPaint(wxPaintEvent& event) {
	SetCurrent(*g_gui.GetGLContext(this));

	if (g_gui.IsRenderingEnabled()) {
		DrawingOptions& options = drawer->getOptions();
		if (screenshot_controller->IsCapturing()) {
			options.SetIngame();
		} else {
			options.Update();
		}

		options.dragging = selection_controller->IsDragging();
		options.boundbox_selection = selection_controller->IsBoundboxSelection();

		if (options.show_preview) {
			animation_timer->Start();
		} else {
			animation_timer->Stop();
		}

		if (!TextRenderer::GetContext()) {
			TextRenderer::Init();
		}

		// BatchRenderer calls removed - MapDrawer handles its own renderers

		drawer->SetupVars();
		drawer->SetupGL();
		drawer->Draw();

		if (screenshot_controller->IsCapturing()) {
			drawer->TakeScreenshot(screenshot_controller->GetBuffer());
		}

		drawer->Release();
		// BatchRenderer::End(); call removed

		// Draw UI (Tooltips) using NanoVG
		if (options.show_tooltips) {
			TextRenderer::BeginFrame(GetSize().x, GetSize().y);
			drawer->DrawTooltips();
			TextRenderer::EndFrame();
		}

		drawer->ClearTooltips();
	}

	// Clean unused textures
	g_gui.gfx.garbageCollection();

	// Swap buffer
	SwapBuffers();

	// FPS tracking and limiting
	frame_pacer.UpdateAndLimit(g_settings.getInteger(Config::FRAME_RATE_LIMIT), g_settings.getBoolean(Config::SHOW_FPS_COUNTER));

	// Send newd node requests
	if (editor.live_manager.GetClient()) {
		editor.live_manager.GetClient()->sendNodeRequests();
	}
}

void MapCanvas::TakeScreenshot(wxFileName path, wxString format) {
	screenshot_controller->TakeScreenshot(path, format);
}

void MapCanvas::ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y) {
	int start_x, start_y;
	GetMapWindow()->GetViewStart(&start_x, &start_y);

	CoordinateMapper::ScreenToMap(screen_x, screen_y, start_x, start_y, zoom, floor, GetContentScaleFactor(), map_x, map_y);
}
#if 0

*map_y = int(start_y + (screen_y * zoom)) / TileSize;
}

if (floor <= GROUND_LAYER) {
	*map_x += GROUND_LAYER - floor;
	*map_y += GROUND_LAYER - floor;
} /* else {
	 *map_x += MAP_MAX_LAYER - floor;
	 *map_y += MAP_MAX_LAYER - floor;
 }*/
}

#endif
void MapCanvas::GetScreenCenter(int* map_x, int* map_y) {
	int width, height;
	GetMapWindow()->GetViewSize(&width, &height);
	return ScreenToMap(width / 2, height / 2, map_x, map_y);
}

Position MapCanvas::GetCursorPosition() const {
	return Position(last_cursor_map_x, last_cursor_map_y, floor);
}

void MapCanvas::UpdatePositionStatus(int x, int y) {
	if (x == -1) {
		x = cursor_x;
	}
	if (y == -1) {
		y = cursor_y;
	}

	int map_x, map_y;
	ScreenToMap(x, y, &map_x, &map_y);

	MapStatusUpdater::Update(editor, map_x, map_y, floor);
}

void MapCanvas::UpdateZoomStatus() {
	ZoomController::UpdateStatus(this);
}

void MapCanvas::OnMouseMove(wxMouseEvent& event) {
	NavigationController::HandleMouseDrag(this, event);

	cursor_x = event.GetX();
	cursor_y = event.GetY();

	int mouse_map_x, mouse_map_y;
	MouseToMap(&mouse_map_x, &mouse_map_y);

	bool map_update = false;
	if (last_cursor_map_x != mouse_map_x || last_cursor_map_y != mouse_map_y || last_cursor_map_z != floor) {
		map_update = true;
	}
	last_cursor_map_x = mouse_map_x;
	last_cursor_map_y = mouse_map_y;
	last_cursor_map_z = floor;

	if (map_update) {
		UpdatePositionStatus(cursor_x, cursor_y);
		UpdateZoomStatus();
		Refresh();
	}

	if (g_gui.IsSelectionMode()) {
		selection_controller->HandleDrag(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	} else { // Drawing mode
		drawing_controller->HandleDrag(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	}
}

void MapCanvas::OnMouseLeftRelease(wxMouseEvent& event) {
	OnMouseActionRelease(event);
}

void MapCanvas::OnMouseLeftClick(wxMouseEvent& event) {
	OnMouseActionClick(event);
}

void MapCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event) {
	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);
	selection_controller->HandleDoubleClick(Position(mouse_map_x, mouse_map_y, floor));
}

void MapCanvas::OnMouseCenterClick(wxMouseEvent& event) {
	if (g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMousePropertiesClick(event);
	} else {
		OnMouseCameraClick(event);
	}
}

void MapCanvas::OnMouseCenterRelease(wxMouseEvent& event) {
	if (g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMousePropertiesRelease(event);
	} else {
		OnMouseCameraRelease(event);
	}
}

void MapCanvas::OnMouseRightClick(wxMouseEvent& event) {
	if (g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMouseCameraClick(event);
	} else {
		OnMousePropertiesClick(event);
	}
}

void MapCanvas::OnMouseRightRelease(wxMouseEvent& event) {
	if (g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMouseCameraRelease(event);
	} else {
		OnMousePropertiesRelease(event);
	}
}

void MapCanvas::OnMouseActionClick(wxMouseEvent& event) {
	SetFocus();

	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	if (event.ControlDown() && event.AltDown()) {
		Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
		BrushSelector::SelectSmartBrush(editor, tile);
	} else if (g_gui.IsSelectionMode()) {
		selection_controller->HandleClick(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	} else if (g_gui.GetCurrentBrush()) { // Drawing mode
		drawing_controller->HandleClick(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	}
	last_click_x = int(event.GetX() * zoom);
	last_click_y = int(event.GetY() * zoom);

	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);
	last_click_abs_x = last_click_x + start_x;
	last_click_abs_y = last_click_y + start_y;

	last_click_map_x = mouse_map_x;
	last_click_map_y = mouse_map_y;
	last_click_map_z = floor;
	g_gui.RefreshView();
	g_gui.UpdateMinimap();
}

void MapCanvas::OnMouseActionRelease(wxMouseEvent& event) {
	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	int move_x = last_click_map_x - mouse_map_x;
	int move_y = last_click_map_y - mouse_map_y;
	int move_z = last_click_map_z - floor;

	if (g_gui.IsSelectionMode()) {
		selection_controller->HandleRelease(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	} else if (g_gui.GetCurrentBrush()) { // Drawing mode
		drawing_controller->HandleRelease(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	}
	g_gui.RefreshView();
	g_gui.UpdateMinimap();
}

void MapCanvas::OnMouseCameraClick(wxMouseEvent& event) {
	SetFocus();

	last_mmb_click_x = event.GetX();
	last_mmb_click_y = event.GetY();
	if (event.ControlDown()) {
		ZoomController::ApplyRelativeZoom(this, 1.0 - zoom);
	} else {
		NavigationController::HandleCameraClick(this, event);
	}
}

void MapCanvas::OnMouseCameraRelease(wxMouseEvent& event) {
	NavigationController::HandleCameraRelease(this, event);
}

void MapCanvas::OnMousePropertiesClick(wxMouseEvent& event) {
	SetFocus();

	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);
	Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);

	if (g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	selection_controller->HandlePropertiesClick(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());
	last_click_y = int(event.GetY() * zoom);

	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);
	last_click_abs_x = last_click_x + start_x;
	last_click_abs_y = last_click_y + start_y;

	last_click_map_x = mouse_map_x;
	last_click_map_y = mouse_map_y;
	g_gui.RefreshView();
}

void MapCanvas::OnMousePropertiesRelease(wxMouseEvent& event) {
	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	if (g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	selection_controller->HandlePropertiesRelease(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown(), event.ControlDown(), event.AltDown());

	popup_menu->Update();
	PopupMenu(popup_menu);

	editor.actionQueue->resetTimer();
	dragging = false;
	boundbox_selection = false;

	last_cursor_map_x = mouse_map_x;
	last_cursor_map_y = mouse_map_y;
	last_cursor_map_z = floor;

	g_gui.RefreshView();
}

void MapCanvas::OnWheel(wxMouseEvent& event) {
	if (event.ControlDown()) {
		NavigationController::HandleWheel(this, event);
	} else if (event.AltDown()) {
		drawing_controller->HandleWheel(event.GetWheelRotation(), event.AltDown(), event.ControlDown());
	} else {
		ZoomController::OnWheel(this, event);
	}

	Refresh();
}

void MapCanvas::OnLoseMouse(wxMouseEvent& event) {
	Refresh();
}

void MapCanvas::OnGainMouse(wxMouseEvent& event) {
	if (!event.LeftIsDown()) {
		dragging = false;
		boundbox_selection = false;
		drawing_controller->Reset();
	}
	if (!event.MiddleIsDown()) {
		screendragging = false;
	}

	Refresh();
}

void MapCanvas::OnKeyDown(wxKeyEvent& event) {
	KeyboardHandler::OnKeyDown(this, event);
}

void MapCanvas::OnKeyUp(wxKeyEvent& event) {
	KeyboardHandler::OnKeyUp(this, event);
}

void MapCanvas::ChangeFloor(int new_floor) {
	NavigationController::ChangeFloor(this, new_floor);
}

void MapCanvas::EnterDrawingMode() {
	dragging = false;
	boundbox_selection = false;
	EndPasting();
	Refresh();
}

void MapCanvas::EnterSelectionMode() {
	drawing_controller->Reset();
	editor.replace_brush = nullptr;
	Refresh();
}

bool MapCanvas::isPasting() const {
	return g_gui.IsPasting();
}

void MapCanvas::StartPasting() {
	g_gui.StartPasting();
}

void MapCanvas::EndPasting() {
	g_gui.EndPasting();
}

void MapCanvas::Reset() {
	cursor_x = 0;
	cursor_y = 0;

	zoom = 1.0;
	floor = GROUND_LAYER;

	dragging = false;
	boundbox_selection = false;
	screendragging = false;
	drawing_controller->Reset();

	editor.replace_brush = nullptr;

	last_click_map_x = -1;
	last_click_map_y = -1;
	last_click_map_z = -1;

	last_mmb_click_x = -1;
	last_mmb_click_y = -1;

	editor.selection.clear();
	editor.actionQueue->clear();
}
