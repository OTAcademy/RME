
void GUI::UpdateAutoborderPreview(Position pos) {
	Brush* brush = GetCurrentBrush();
	if (brush && brush->isDoodad()) {
		return;
	}

	if (IsDrawingMode() && brush && brush->needBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		g_autoborder_preview.Update(GetCurrentMap().getEditor(), pos);
		secondary_map = g_autoborder_preview.GetBufferMap();
	} else {
		if (secondary_map == g_autoborder_preview.GetBufferMap()) {
			g_autoborder_preview.Clear();
			secondary_map = nullptr;
		}
	}
}
