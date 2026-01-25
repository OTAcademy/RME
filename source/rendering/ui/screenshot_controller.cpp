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
#include "rendering/ui/screenshot_controller.h"
#include "rendering/ui/map_display.h"
#include "rendering/io/screenshot_saver.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"

ScreenshotController::ScreenshotController(MapCanvas* canvas) :
	canvas(canvas) {
	screenshot_saver = std::make_unique<ScreenshotSaver>();
}

ScreenshotController::~ScreenshotController() {
}

bool ScreenshotController::IsCapturing() const {
	return screenshot_saver->IsCapturing();
}

uint8_t* ScreenshotController::GetBuffer() {
	return screenshot_saver->GetBuffer();
}

void ScreenshotController::TakeScreenshot(const wxFileName& path, const wxString& format) {
	int view_scroll_x, view_scroll_y;
	int screensize_x, screensize_y;
	canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

	screenshot_saver->PrepareCapture(screensize_x, screensize_y);

	// Draw the window
	canvas->Refresh();
	canvas->Update(); // Forces immediate redraws the window.

	// Buffer should now contain the screenbuffer

	// We need to re-get view size as it might be used in SaveCapture logic indirectly or just to be safe
	// But in original code: static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);
	// In MapCanvas::GetViewBox we already do that.

	// Delegate saving to ScreenshotSaver
	wxString result = screenshot_saver->SaveCapture(path, format, screensize_x, screensize_y);

	if (result.StartsWith("Error:")) {
		DialogUtil::PopupDialog("Screenshot Error", result, wxOK);
	} else {
		g_gui.SetStatusText(result);
	}

	canvas->Refresh();

	screenshot_saver->Cleanup();
}
