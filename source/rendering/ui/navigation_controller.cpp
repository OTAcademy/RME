//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/ui/navigation_controller.h"
#include "rendering/ui/map_display.h"
#include "map_window.h"
#include "gui.h"
#include <algorithm>
#include <cmath>

extern GUI g_gui;

void NavigationController::HandleArrowKeys(MapCanvas* canvas, wxKeyEvent& event) {
	int start_x, start_y;
	static_cast<MapWindow*>(canvas->GetParent())->GetViewStart(&start_x, &start_y);

	int tiles = 3;
	if (event.ControlDown()) {
		tiles = 10;
	} else if (canvas->zoom == 1.0) {
		tiles = 1;
	}

	int keycode = event.GetKeyCode();
	if (keycode == WXK_NUMPAD_UP || keycode == WXK_UP) {
		static_cast<MapWindow*>(canvas->GetParent())->Scroll(start_x, int(start_y - TileSize * tiles * canvas->zoom));
	} else if (keycode == WXK_NUMPAD_DOWN || keycode == WXK_DOWN) {
		static_cast<MapWindow*>(canvas->GetParent())->Scroll(start_x, int(start_y + TileSize * tiles * canvas->zoom));
	} else if (keycode == WXK_NUMPAD_LEFT || keycode == WXK_LEFT) {
		static_cast<MapWindow*>(canvas->GetParent())->Scroll(int(start_x - TileSize * tiles * canvas->zoom), start_y);
	} else if (keycode == WXK_NUMPAD_RIGHT || keycode == WXK_RIGHT) {
		static_cast<MapWindow*>(canvas->GetParent())->Scroll(int(start_x + TileSize * tiles * canvas->zoom), start_y);
	}

	canvas->UpdatePositionStatus();
	canvas->Refresh();
}

void NavigationController::HandleMouseDrag(MapCanvas* canvas, wxMouseEvent& event) {
	if (canvas->screendragging) {
		static_cast<MapWindow*>(canvas->GetParent())->ScrollRelative(int(g_settings.getFloat(Config::SCROLL_SPEED) * canvas->zoom * (event.GetX() - canvas->cursor_x)), int(g_settings.getFloat(Config::SCROLL_SPEED) * canvas->zoom * (event.GetY() - canvas->cursor_y)));
		canvas->Refresh();
	}
}

void NavigationController::HandleCameraClick(MapCanvas* canvas, wxMouseEvent& event) {
	canvas->SetFocus();

	canvas->last_mmb_click_x = event.GetX();
	canvas->last_mmb_click_y = event.GetY();

	// Control logic is handled by ZoomController in OnMouseCameraClick wrapper usually, but if extracted here:
	// But MapCanvas::OnMouseCameraClick has explicit Control check for Reset Zoom.
	// If NavigationController handles Click, it should know about Zoom Reset?
	// Or MapCanvas delegates only "Navigation logic" here?
	// MapCanvas::OnMouseCameraClick implementation:
	/*
	if (event.ControlDown()) {
		// Zoom Reset logic ...
	} else {
		screendragging = true;
	}
	*/
	// So purely navigation part is just screendragging = true.
	// But HandleCameraClick implies handling the event.
	// I'll put the screendragging = true logic here.
	// MapCanvas should check ControlDown first.

	canvas->screendragging = true;
}

void NavigationController::HandleCameraRelease(MapCanvas* canvas, wxMouseEvent& event) {
	canvas->SetFocus();
	canvas->screendragging = false;
	if (event.ControlDown()) {
		// ...
		// Haven't moved much, it's a click!
	} else if (canvas->last_mmb_click_x > event.GetX() - 3 && canvas->last_mmb_click_x < event.GetX() + 3 && canvas->last_mmb_click_y > event.GetY() - 3 && canvas->last_mmb_click_y < event.GetY() + 3) {
		int screensize_x, screensize_y;
		static_cast<MapWindow*>(canvas->GetParent())->GetViewSize(&screensize_x, &screensize_y);
		static_cast<MapWindow*>(canvas->GetParent())->ScrollRelative(int(canvas->zoom * (2 * canvas->cursor_x - screensize_x)), int(canvas->zoom * (2 * canvas->cursor_y - screensize_y)));
		canvas->Refresh();
	}
}

void NavigationController::ChangeFloor(MapCanvas* canvas, int new_floor) {
	new_floor = std::clamp(new_floor, 0, MAP_LAYERS - 1);
	int old_floor = canvas->floor;
	canvas->floor = new_floor;
	if (old_floor != new_floor) {
		canvas->UpdatePositionStatus();
		g_gui.root->UpdateFloorMenu();
		g_gui.UpdateMinimap(true);
	}
	canvas->Refresh();
}

void NavigationController::HandleWheel(MapCanvas* canvas, wxMouseEvent& event) {
	if (event.ControlDown()) {
		static double diff = 0.0;
		diff += event.GetWheelRotation();
		if (diff <= 1.0 || diff >= 1.0) {
			if (diff < 0.0) {
				ChangeFloor(canvas, canvas->floor - 1);
			} else {
				ChangeFloor(canvas, canvas->floor + 1);
			}
			diff = 0.0;
		}
		canvas->UpdatePositionStatus();
	}
}
