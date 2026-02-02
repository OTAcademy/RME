
#include "ui/gui.h"
#include "brushes/brush.h"
#include "app/settings.h"
#include "editor/editor.h"
#include "map/map.h"
#include "map/position.h"
#include "brushes/managers/autoborder_preview_manager.h"

void GUI::UpdateAutoborderPreview(Position pos) {
	Brush* brush = GetCurrentBrush();
	if (brush && brush->isDoodad()) {
		if (secondary_map == g_autoborder_preview.GetBufferMap()) {
			g_autoborder_preview.Clear();
			secondary_map = nullptr;
		}
		return;
	}

	// Check for editor to ensure we don't dereference a null pointer
	Editor* editor = GetCurrentEditor();
	if (IsDrawingMode() && brush && brush->needBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC) && editor) {
		g_autoborder_preview.Update(*editor, pos, wxGetKeyState(WXK_ALT));
		secondary_map = g_autoborder_preview.GetBufferMap();
	} else {
		if (secondary_map == g_autoborder_preview.GetBufferMap()) {
			g_autoborder_preview.Clear();
			secondary_map = nullptr;
		}
	}
}
