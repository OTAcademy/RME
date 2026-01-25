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
#include "rendering/ui/keyboard_handler.h"
#include "rendering/ui/zoom_controller.h"
#include "rendering/ui/navigation_controller.h"
#include "rendering/ui/map_display.h"
#include "ui/map_window.h"
#include "ui/gui.h"
#include "editor/hotkey_manager.h"
#include "editor/editor.h"
#include "brushes/brush.h"

void KeyboardHandler::OnKeyDown(MapCanvas* canvas, wxKeyEvent& event) {
	switch (event.GetKeyCode()) {
		case WXK_NUMPAD_ADD:
		case WXK_PAGEUP:
		case WXK_NUMPAD_SUBTRACT:
		case WXK_PAGEDOWN: {
			HandleFloorChange(canvas, event.GetKeyCode());
			break;
		}
		case WXK_NUMPAD_MULTIPLY:
		case WXK_NUMPAD_DIVIDE: {
			HandleZoomKeys(canvas, event.GetKeyCode());
			break;
		}
		case '[':
		case '+':
		case ']':
		case '-': {
			HandleBrushSizeChange(canvas, event.GetKeyCode());
			break;
		}
		case WXK_NUMPAD_UP:
		case WXK_UP:
		case WXK_NUMPAD_DOWN:
		case WXK_DOWN:
		case WXK_NUMPAD_LEFT:
		case WXK_LEFT:
		case WXK_NUMPAD_RIGHT:
		case WXK_RIGHT: {
			HandleArrowNavigation(canvas, event);
			break;
		}
		case WXK_SPACE: {
			if (event.ControlDown()) {
				g_gui.FillDoodadPreviewBuffer();
				g_gui.RefreshView();
			} else {
				g_gui.SwitchMode();
			}
			break;
		}
		case WXK_TAB: {
			if (event.ShiftDown()) {
				g_gui.CycleTab(false);
			} else {
				g_gui.CycleTab(true);
			}
			break;
		}
		case WXK_DELETE: {
			canvas->editor.destroySelection();
			g_gui.RefreshView();
			break;
		}
		case 'z':
		case 'Z':
		case 'x':
		case 'X': {
			HandleBrushVariation(canvas, event.GetKeyCode());
			break;
		}
		case 'q':
		case 'Q': {
			g_gui.SelectPreviousBrush();
			break;
		}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			HandleHotkeys(canvas, event);
			break;
		}
		case 'd':
		case 'D': {
			canvas->keyCode = WXK_CONTROL_D;
			break;
		}
		default: {
			event.Skip();
			break;
		}
	}
}

void KeyboardHandler::OnKeyUp(MapCanvas* canvas, wxKeyEvent& event) {
	canvas->keyCode = WXK_NONE;
}

void KeyboardHandler::HandleFloorChange(MapCanvas* canvas, int keycode) {
	if (keycode == WXK_NUMPAD_ADD || keycode == WXK_PAGEUP) {
		g_gui.ChangeFloor(canvas->floor - 1);
	} else {
		g_gui.ChangeFloor(canvas->floor + 1);
	}
}

void KeyboardHandler::HandleZoomKeys(MapCanvas* canvas, int keycode) {
	double diff = (keycode == WXK_NUMPAD_MULTIPLY) ? -0.3 : 0.3;
	ZoomController::ApplyRelativeZoom(canvas, diff);
}

void KeyboardHandler::HandleArrowNavigation(MapCanvas* canvas, wxKeyEvent& event) {
	NavigationController::HandleArrowKeys(canvas, event);
}

void KeyboardHandler::HandleBrushSizeChange(MapCanvas* canvas, int keycode) {
	if (keycode == '[' || keycode == '+') {
		g_gui.IncreaseBrushSize();
	} else {
		g_gui.DecreaseBrushSize();
	}
	canvas->Refresh();
}

void KeyboardHandler::HandleBrushVariation(MapCanvas* canvas, int keycode) {
	int nv = g_gui.GetBrushVariation();
	if (keycode == 'z' || keycode == 'Z') {
		--nv;
		if (nv < 0) {
			nv = max(0, (g_gui.GetCurrentBrush() ? g_gui.GetCurrentBrush()->getMaxVariation() - 1 : 0));
		}
	} else {
		++nv;
		if (nv >= (g_gui.GetCurrentBrush() ? g_gui.GetCurrentBrush()->getMaxVariation() : 0)) {
			nv = 0;
		}
	}
	g_gui.SetBrushVariation(nv);
	g_gui.RefreshView();
}

void KeyboardHandler::HandleHotkeys(MapCanvas* canvas, wxKeyEvent& event) {
	int index = event.GetKeyCode() - '0';
	if (event.ControlDown()) {
		Hotkey hk;
		if (g_gui.IsSelectionMode()) {
			int view_start_x, view_start_y;
			static_cast<MapWindow*>(canvas->GetParent())->GetViewStart(&view_start_x, &view_start_y);
			int view_start_map_x = view_start_x / TileSize, view_start_map_y = view_start_y / TileSize;

			int view_screensize_x, view_screensize_y;
			static_cast<MapWindow*>(canvas->GetParent())->GetViewSize(&view_screensize_x, &view_screensize_y);

			int map_x = int(view_start_map_x + (view_screensize_x * canvas->zoom) / TileSize / 2);
			int map_y = int(view_start_map_y + (view_screensize_y * canvas->zoom) / TileSize / 2);

			hk = Hotkey(Position(map_x, map_y, canvas->floor));
		} else if (g_gui.GetCurrentBrush()) {
			hk = Hotkey(g_gui.GetCurrentBrush());
		} else {
			return;
		}
		g_hotkeys.SetHotkey(index, hk);
	} else {
		Hotkey hk = g_hotkeys.GetHotkey(index);
		if (hk.IsPosition()) {
			g_gui.SetSelectionMode();

			int map_x = hk.GetPosition().x;
			int map_y = hk.GetPosition().y;
			int map_z = hk.GetPosition().z;

			static_cast<MapWindow*>(canvas->GetParent())->Scroll(TileSize * map_x, TileSize * map_y, true);
			canvas->floor = map_z;

			g_gui.SetStatusText("Used hotkey " + i2ws(index));
			g_gui.RefreshView();
		} else if (hk.IsBrush()) {
			g_gui.SetDrawingMode();

			std::string name = hk.GetBrushname();
			Brush* brush = g_brushes.getBrush(name);
			if (brush == nullptr) {
				g_gui.SetStatusText("Brush \"" + wxstr(name) + "\" not found");
				return;
			}

			if (!g_gui.SelectBrush(brush)) {
				g_gui.SetStatusText("Brush \"" + wxstr(name) + "\" is not in any palette");
				return;
			}

			g_gui.SetStatusText("Used hotkey " + i2ws(index));
			g_gui.RefreshView();
		} else {
			g_gui.SetStatusText("Unassigned hotkey " + i2ws(index));
		}
	}
}
