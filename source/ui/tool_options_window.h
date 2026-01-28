#ifndef RME_UI_TOOL_OPTIONS_WINDOW_H_
#define RME_UI_TOOL_OPTIONS_WINDOW_H_

#include "app/main.h"
#include "palette/palette_common.h"

class BrushSizePanel;
class BrushToolPanel;
class BrushThicknessPanel;

class ToolOptionsWindow : public wxPanel {
public:
	ToolOptionsWindow(wxWindow* parent);
	~ToolOptionsWindow();

	void SetPaletteType(PaletteType type);
	void UpdateBrushSize(BrushShape shape, int size);

	// Updates internal panels based on settings
	void ReloadSettings();

protected:
	BrushSizePanel* size_panel;
	BrushToolPanel* tool_panel;
	BrushThicknessPanel* thickness_panel;

	PaletteType current_type;
	wxSizer* main_sizer;
};

#endif
