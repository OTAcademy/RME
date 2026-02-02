#ifndef RME_UI_TOOL_OPTIONS_WINDOW_H_
#define RME_UI_TOOL_OPTIONS_WINDOW_H_

#include "app/main.h"
#include "palette/palette_common.h"

class ToolOptionsSurface;

class ToolOptionsWindow : public wxPanel {
public:
	ToolOptionsWindow(wxWindow* parent);
	~ToolOptionsWindow();

	void SetPaletteType(PaletteType type);
	void UpdateBrushSize(BrushShape shape, int size);
	void ReloadSettings();

protected:
	wxSizer* main_sizer;
	ToolOptionsSurface* surface;
};

#endif
