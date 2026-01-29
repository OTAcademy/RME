#ifndef RME_PALETTE_PANELS_BRUSH_SIZE_PANEL_H_
#define RME_PALETTE_PANELS_BRUSH_SIZE_PANEL_H_

#include "palette/palette_common.h"
#include "ui/dcbutton.h"

class BrushSizePanel : public PalettePanel {
public:
	BrushSizePanel(wxWindow* parent);
	~BrushSizePanel() override { }

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents() override;
	// Loads the currently displayed page
	void LoadCurrentContents() override;
	// Loads all content in this panel
	void LoadAllContents() override;

	wxString GetName() const override;
	void SetToolbarIconSize(bool large) override;

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size) override;
	// Called when this page is displayed
	void OnSwitchIn() override;

	// wxWidgets event handling
	void OnClickSquareBrush(wxCommandEvent& event);
	void OnClickCircleBrush(wxCommandEvent& event);

	void OnClickBrushSize(int which);
	void OnClickBrushSize0(wxCommandEvent& event) {
		OnClickBrushSize(0);
	}
	void OnClickBrushSize1(wxCommandEvent& event) {
		OnClickBrushSize(1);
	}
	void OnClickBrushSize2(wxCommandEvent& event) {
		OnClickBrushSize(2);
	}
	void OnClickBrushSize4(wxCommandEvent& event) {
		OnClickBrushSize(4);
	}
	void OnClickBrushSize6(wxCommandEvent& event) {
		OnClickBrushSize(6);
	}
	void OnClickBrushSize8(wxCommandEvent& event) {
		OnClickBrushSize(8);
	}
	void OnClickBrushSize11(wxCommandEvent& event) {
		OnClickBrushSize(11);
	}

protected:
	bool loaded = false;
	bool large_icons = true;

	DCButton* brushshapeSquareButton = nullptr;
	DCButton* brushshapeCircleButton = nullptr;

	DCButton* brushsize0Button = nullptr;
	DCButton* brushsize1Button = nullptr;
	DCButton* brushsize2Button = nullptr;
	DCButton* brushsize4Button = nullptr;
	DCButton* brushsize6Button = nullptr;
	DCButton* brushsize8Button = nullptr;
	DCButton* brushsize11Button = nullptr;

	DECLARE_EVENT_TABLE()
};

#endif
