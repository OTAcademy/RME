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

#include <spdlog/spdlog.h>

#include "rendering/core/graphics.h"
#include "editor/editor.h"
#include "map/map.h"

#include "ui/gui.h"
#include "rendering/ui/map_display.h"
#include "rendering/ui/minimap_window.h"

#include "rendering/drawers/minimap_drawer.h"

BEGIN_EVENT_TABLE(MinimapWindow, wxPanel)
EVT_LEFT_DOWN(MinimapWindow::OnMouseClick)
EVT_SIZE(MinimapWindow::OnSize)
EVT_PAINT(MinimapWindow::OnPaint)
EVT_ERASE_BACKGROUND(MinimapWindow::OnEraseBackground)
EVT_CLOSE(MinimapWindow::OnClose)
EVT_TIMER(wxID_ANY, MinimapWindow::OnDelayedUpdate)
EVT_KEY_DOWN(MinimapWindow::OnKey)
END_EVENT_TABLE()

// Helper to create attributes
static wxGLAttributes& GetCoreProfileAttributes() {
	static wxGLAttributes vAttrs = []() {
		wxGLAttributes a;
		a.PlatformDefaults().Defaults().RGBA().DoubleBuffer().Depth(24).Stencil(8).EndList();
		return a;
	}();
	return vAttrs;
}

MinimapWindow::MinimapWindow(wxWindow* parent) :
	wxGLCanvas(parent, GetCoreProfileAttributes(), wxID_ANY, wxDefaultPosition, wxSize(205, 130)),
	update_timer(this),
	context(nullptr) {
	spdlog::info("MinimapWindow::MinimapWindow - Creating context");
	context = new wxGLContext(this);
	if (!context->IsOK()) {
		spdlog::error("MinimapWindow::MinimapWindow - Context creation failed");
	}
	drawer = std::make_unique<MinimapDrawer>();
}

MinimapWindow::~MinimapWindow() {
	delete context;
}

void MinimapWindow::OnSize(wxSizeEvent& event) {
	Refresh();
}

void MinimapWindow::OnClose(wxCloseEvent&) {
	g_gui.DestroyMinimap();
}

void MinimapWindow::DelayedUpdate() {
	// We only updated the window AFTER actions have taken place, that
	// way we don't waste too much performance on updating this window
	update_timer.Start(g_settings.getInteger(Config::MINIMAP_UPDATE_DELAY), true);
}

void MinimapWindow::OnDelayedUpdate(wxTimerEvent& event) {
	Refresh();
}

void MinimapWindow::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this); // Required for wxGLCanvas

	// spdlog::info("MinimapWindow::OnPaint");

	if (!context) {
		spdlog::error("MinimapWindow::OnPaint - No context!");
		return;
	}

	SetCurrent(*context);

	static bool gladInitialized = false;
	if (!gladInitialized) {
		spdlog::info("MinimapWindow::OnPaint - Initializing GLAD");
		if (!gladLoadGL()) {
			spdlog::error("MinimapWindow::OnPaint - Failed to load GLAD");
		} else {
			spdlog::info("MinimapWindow::OnPaint - GLAD loaded. GL Version: {}", (char*)glGetString(GL_VERSION));
		}
		gladInitialized = true;
	}

	if (!g_gui.IsEditorOpen()) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SwapBuffers();
		return;
	}
	Editor& editor = *g_gui.GetCurrentEditor();
	MapCanvas* canvas = g_gui.GetCurrentMapTab()->GetCanvas();

	// Mock dc passed to Draw, unused by new GL implementation
	drawer->Draw(dc, GetSize(), editor, canvas);

	SwapBuffers();
}

void MinimapWindow::OnMouseClick(wxMouseEvent& event) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}
	int new_map_x, new_map_y;
	drawer->ScreenToMap(event.GetX(), event.GetY(), new_map_x, new_map_y);

	g_gui.SetScreenCenterPosition(Position(new_map_x, new_map_y, g_gui.GetCurrentFloor()));
	Refresh();
	g_gui.RefreshView();
}

void MinimapWindow::OnKey(wxKeyEvent& event) {
	if (g_gui.GetCurrentTab() != nullptr) {
		g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
	}
}
