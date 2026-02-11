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

#define NANOVG_GL3
#include <nanovg.h>
#include <nanovg_gl.h>

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
	context(nullptr),
	nvg(nullptr, NVGDeleter()) {
	spdlog::info("MinimapWindow::MinimapWindow - Creating context");
	context = std::make_unique<wxGLContext>(this);
	if (!context->IsOK()) {
		spdlog::error("MinimapWindow::MinimapWindow - Context creation failed");
	}
	SetToolTip("Click to move camera");
	drawer = std::make_unique<MinimapDrawer>();

	Bind(wxEVT_LEFT_DOWN, &MinimapWindow::OnMouseClick, this);
	Bind(wxEVT_SIZE, &MinimapWindow::OnSize, this);
	Bind(wxEVT_PAINT, &MinimapWindow::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &MinimapWindow::OnEraseBackground, this);
	Bind(wxEVT_CLOSE_WINDOW, &MinimapWindow::OnClose, this);
	Bind(wxEVT_TIMER, &MinimapWindow::OnDelayedUpdate, this, wxID_ANY);
	Bind(wxEVT_KEY_DOWN, &MinimapWindow::OnKey, this);
}

MinimapWindow::~MinimapWindow() {
	if (context && nvg) {
		SetCurrent(*context);
		nvg.reset();
	}
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
	wxPaintDC dc(this); // validates the paint event

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

	if (!nvg) {
		// Minimap uses a separate NanoVG context to avoid state interference with the main
		// TextRenderer, as the minimap window has its own GL context and lifecycle.
		nvg.reset(nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES));
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

	// Glass Overlay
	NVGcontext* vg = nvg.get();
	if (vg) {
		int w, h;
		GetClientSize(&w, &h);
		nvgBeginFrame(vg, w, h, GetContentScaleFactor());

		// Subtle glass border
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 1.5f, 1.5f, w - 3.0f, h - 3.0f, 4.0f);
		nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 60));
		nvgStrokeWidth(vg, 2.0f);
		nvgStroke(vg);

		// Inner glow
		NVGpaint glow = nvgBoxGradient(vg, 0, 0, w, h, 4.0f, 20.0f, nvgRGBA(255, 255, 255, 10), nvgRGBA(0, 0, 0, 40));
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, w, h, 4.0f);
		nvgFillPaint(vg, glow);
		nvgFill(vg);

		nvgEndFrame(vg);
	}

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
