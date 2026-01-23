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

#include <sstream>
#include <time.h>
#include <thread>
#include <chrono>
#include <wx/wfstream.h>

#include "gui.h"
#include "editor.h"
#include "brush.h"
#include "sprites.h"
#include "map.h"
#include "tile.h"
#include "old_properties_window.h"
#include "properties_window.h"
#include "tileset_window.h"
#include "palette_window.h"
#include "palette_window.h"
#include "rendering/utilities/fps_counter.h"
#include "rendering/ui/screenshot_controller.h"
#include "rendering/utilities/tile_describer.h"
#include "rendering/core/coordinate_mapper.h"
#include "rendering/ui/map_display.h"
#include "rendering/ui/map_status_updater.h"
#include "rendering/map_drawer.h"
#include "application.h"
#include "live_server.h"
#include "browse_tile_window.h"
#include "dialog_helper.h"
#include "animation_timer.h"
#include "map_popup_menu.h"
#include "brush_utility.h"
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

#include "doodad_brush.h"
#include "house_exit_brush.h"
#include "house_brush.h"
#include "wall_brush.h"
#include "spawn_brush.h"
#include "creature_brush.h"
#include "ground_brush.h"
#include "waypoint_brush.h"
#include "raw_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"

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
EVT_MENU(MAP_POPUP_MENU_CUT, MapCanvas::OnCut)
EVT_MENU(MAP_POPUP_MENU_COPY, MapCanvas::OnCopy)
EVT_MENU(MAP_POPUP_MENU_COPY_POSITION, MapCanvas::OnCopyPosition)
EVT_MENU(MAP_POPUP_MENU_PASTE, MapCanvas::OnPaste)
EVT_MENU(MAP_POPUP_MENU_DELETE, MapCanvas::OnDelete)
//----
EVT_MENU(MAP_POPUP_MENU_COPY_SERVER_ID, MapCanvas::OnCopyServerId)
EVT_MENU(MAP_POPUP_MENU_COPY_CLIENT_ID, MapCanvas::OnCopyClientId)
EVT_MENU(MAP_POPUP_MENU_COPY_NAME, MapCanvas::OnCopyName)
// ----
EVT_MENU(MAP_POPUP_MENU_ROTATE, MapCanvas::OnRotateItem)
EVT_MENU(MAP_POPUP_MENU_GOTO, MapCanvas::OnGotoDestination)
EVT_MENU(MAP_POPUP_MENU_SWITCH_DOOR, MapCanvas::OnSwitchDoor)
// ----
EVT_MENU(MAP_POPUP_MENU_SELECT_RAW_BRUSH, MapCanvas::OnSelectRAWBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_GROUND_BRUSH, MapCanvas::OnSelectGroundBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_DOODAD_BRUSH, MapCanvas::OnSelectDoodadBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_COLLECTION_BRUSH, MapCanvas::OnSelectCollectionBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_DOOR_BRUSH, MapCanvas::OnSelectDoorBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_WALL_BRUSH, MapCanvas::OnSelectWallBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_CARPET_BRUSH, MapCanvas::OnSelectCarpetBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_TABLE_BRUSH, MapCanvas::OnSelectTableBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_CREATURE_BRUSH, MapCanvas::OnSelectCreatureBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_SPAWN_BRUSH, MapCanvas::OnSelectSpawnBrush)
EVT_MENU(MAP_POPUP_MENU_SELECT_HOUSE_BRUSH, MapCanvas::OnSelectHouseBrush)
EVT_MENU(MAP_POPUP_MENU_MOVE_TO_TILESET, MapCanvas::OnSelectMoveTo)
// ----
EVT_MENU(MAP_POPUP_MENU_PROPERTIES, MapCanvas::OnProperties)
// ----
EVT_MENU(MAP_POPUP_MENU_BROWSE_TILE, MapCanvas::OnBrowseTile)
END_EVENT_TABLE()

bool MapCanvas::processed[] = { 0 };

MapCanvas::MapCanvas(MapWindow* parent, Editor& editor, int* attriblist) :
	wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
	editor(editor),
	floor(GROUND_LAYER),
	zoom(1.0),
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
	static_cast<MapWindow*>(GetParent())->GetViewSize(screensize_x, screensize_y);
	static_cast<MapWindow*>(GetParent())->GetViewStart(view_scroll_x, view_scroll_y);
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

		// Calculate current house ID for highlighting
		Brush* brush = g_gui.GetCurrentBrush();
		options.current_house_id = 0;
		if (brush) {
			if (brush->isHouse()) {
				options.current_house_id = brush->asHouse()->getHouseID();
			} else if (brush->isHouseExit()) {
				options.current_house_id = brush->asHouseExit()->getHouseID();
			}
		}

		options.dragging = selection_controller->IsDragging();
		options.boundbox_selection = selection_controller->IsBoundboxSelection();

		if (options.show_preview) {
			animation_timer->Start();
		} else {
			animation_timer->Stop();
		}

		drawer->SetupVars();
		drawer->SetupGL();
		drawer->Draw();

		if (screenshot_controller->IsCapturing()) {
			drawer->TakeScreenshot(screenshot_controller->GetBuffer());
		}

		drawer->Release();
	}

	// Clean unused textures
	g_gui.gfx.garbageCollection();

	// Swap buffer
	SwapBuffers();

	// FPS tracking and limiting
	fps_counter.LimitFPS(g_settings.getInteger(Config::FRAME_RATE_LIMIT));
	fps_counter.Update();

	// Display FPS on status bar if enabled
	if (g_settings.getBoolean(Config::SHOW_FPS_COUNTER)) {
		// Display FPS on status bar if enabled
		if (g_settings.getBoolean(Config::SHOW_FPS_COUNTER) && fps_counter.HasChanged()) {
			g_gui.root->SetStatusText(fps_counter.GetStatusString(), 0);
		}
	}

	// Send newd node requests
	editor.SendNodeRequests();
}

void MapCanvas::TakeScreenshot(wxFileName path, wxString format) {
	screenshot_controller->TakeScreenshot(path, format);
}

void MapCanvas::ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y) {
	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

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
	static_cast<MapWindow*>(GetParent())->GetViewSize(&width, &height);
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
		static double diff = 0.0;
		diff += event.GetWheelRotation();
		if (diff <= 1.0 || diff >= 1.0) {
			if (diff < 0.0) {
				g_gui.IncreaseBrushSize();
			} else {
				g_gui.DecreaseBrushSize();
			}
			diff = 0.0;
		}
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

void MapCanvas::OnCopy(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copy(editor, GetFloor());
}

void MapCanvas::OnCut(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::cut(editor, GetFloor());
}

void MapCanvas::OnPaste(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::paste();
}

void MapCanvas::OnDelete(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::doDelete(editor);
}

void MapCanvas::OnCopyPosition(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyPosition(editor.selection);
}

void MapCanvas::OnCopyServerId(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyServerId(editor.selection);
}

void MapCanvas::OnCopyClientId(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyClientId(editor.selection);
}

void MapCanvas::OnCopyName(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyName(editor.selection);
}

void MapCanvas::OnBrowseTile(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::BrowseTile(editor, cursor_x, cursor_y);
}

void MapCanvas::OnRotateItem(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::RotateItem(editor);
}

void MapCanvas::OnGotoDestination(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::GotoDestination(editor);
}

void MapCanvas::OnSwitchDoor(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::SwitchDoor(editor);
}

void MapCanvas::OnSelectRAWBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectRAWBrush(editor.selection);
}

void MapCanvas::OnSelectGroundBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectGroundBrush(editor.selection);
}

void MapCanvas::OnSelectDoodadBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectDoodadBrush(editor.selection);
}

void MapCanvas::OnSelectDoorBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectDoorBrush(editor.selection);
}

void MapCanvas::OnSelectWallBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectWallBrush(editor.selection);
}

void MapCanvas::OnSelectCarpetBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCarpetBrush(editor.selection);
}

void MapCanvas::OnSelectTableBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectTableBrush(editor.selection);
}

void MapCanvas::OnSelectHouseBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectHouseBrush(editor, editor.selection);
}

void MapCanvas::OnSelectCollectionBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCollectionBrush(editor.selection);
}

void MapCanvas::OnSelectCreatureBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCreatureBrush(editor.selection);
}

void MapCanvas::OnSelectSpawnBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectSpawnBrush();
}

void MapCanvas::OnSelectMoveTo(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::SelectMoveTo(editor);
}

void MapCanvas::OnProperties(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::OpenProperties(editor);
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
