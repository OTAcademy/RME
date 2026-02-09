
#include "ui/gui.h"
#include "brushes/brush.h"
#include "app/settings.h"
#include "editor/editor.h"
#include "map/map.h"
#include "map/position.h"
#include "brushes/managers/autoborder_preview_manager.h"

#include "ui/map_tab.h"

void GUI::UpdateAutoborderPreview(Position pos) {
	MapTab* mapTab = GetCurrentMapTab();
	Brush* brush = GetCurrentBrush();
	if (brush && brush->isDoodad()) {
		if (mapTab && mapTab->GetSession()->secondary_map == g_autoborder_preview.GetBufferMap()) {
			g_autoborder_preview.Clear();
			mapTab->GetSession()->secondary_map = nullptr;
		}
		return;
	}

	// Check for editor to ensure we don't dereference a null pointer
	if (IsDrawingMode() && brush && brush->needBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::SHOW_AUTOBORDER_PREVIEW) && mapTab) {
		g_autoborder_preview.Update(*(mapTab->GetEditor()), pos, wxGetKeyState(WXK_ALT));
		mapTab->GetSession()->secondary_map = g_autoborder_preview.GetBufferMap();
	} else if (mapTab) {
		if (mapTab->GetSession()->secondary_map == g_autoborder_preview.GetBufferMap()) {
			g_autoborder_preview.Clear();
			mapTab->GetSession()->secondary_map = nullptr;
		}
	}
}
