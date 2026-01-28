#ifndef RME_PALETTE_PANELS_BRUSH_THICKNESS_PANEL_H_
#define RME_PALETTE_PANELS_BRUSH_THICKNESS_PANEL_H_

#include "palette/palette_common.h"

class BrushThicknessPanel : public PalettePanel {
public:
	BrushThicknessPanel(wxWindow* parent);
	~BrushThicknessPanel() override;

	// Interface
	wxString GetName() const override;

	// Called when this page is displayed
	void OnSwitchIn() override;

	// wxWidgets event handling
	void OnScroll(wxScrollEvent& event);
	void OnClickCustomThickness(wxCommandEvent& event);

public:
	wxSlider* slider = nullptr;
	wxCheckBox* use_button = nullptr;

	DECLARE_EVENT_TABLE()
};

#endif
