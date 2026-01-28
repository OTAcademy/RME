#ifndef RME_PALETTE_PANELS_BRUSH_TOOL_PANEL_H_
#define RME_PALETTE_PANELS_BRUSH_TOOL_PANEL_H_

#include "palette/palette_common.h"
#include "palette/controls/brush_button.h"
#include "app/settings.h"

class BrushToolPanel : public PalettePanel {
public:
	BrushToolPanel(wxWindow* parent);
	~BrushToolPanel() override;

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents() override;
	// Loads the currently displayed page
	void LoadCurrentContents() override;
	// Loads all content in this panel
	void LoadAllContents() override;

	wxString GetName() const override;
	void SetToolbarIconSize(bool large) override;

	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const override;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush) override;

	// Called when this page is displayed
	void OnSwitchIn() override;

	// wxWidgets event handling
	void OnClickGravelButton(wxCommandEvent& event);
	void OnClickEraserButton(wxCommandEvent& event);
	// ----
	void OnClickNormalDoorButton(wxCommandEvent& event);
	void OnClickLockedDoorButton(wxCommandEvent& event);
	void OnClickMagicDoorButton(wxCommandEvent& event);
	void OnClickQuestDoorButton(wxCommandEvent& event);
	void OnClickHatchDoorButton(wxCommandEvent& event);
	void OnClickWindowDoorButton(wxCommandEvent& event);
	void OnClickNormalAltDoorButton(wxCommandEvent& event);
	void OnClickArchwayDoorButton(wxCommandEvent& event);
	// ----
	void OnClickPZBrushButton(wxCommandEvent& event);
	void OnClickNOPVPBrushButton(wxCommandEvent& event);
	void OnClickNoLogoutBrushButton(wxCommandEvent& event);
	void OnClickPVPZoneBrushButton(wxCommandEvent& event);
	// ----
	void OnClickLockDoorCheckbox(wxCommandEvent& event);

public:
	void DeselectAll();

	bool loaded = false;
	bool large_icons = true;

	BrushButton* optionalBorderButton = nullptr;
	BrushButton* eraserButton = nullptr;
	// ----
	BrushButton* normalDoorButton = nullptr;
	BrushButton* lockedDoorButton = nullptr;
	BrushButton* magicDoorButton = nullptr;
	BrushButton* questDoorButton = nullptr;
	BrushButton* hatchDoorButton = nullptr;
	BrushButton* windowDoorButton = nullptr;
	BrushButton* normalDoorAltButton = nullptr;
	BrushButton* archwayDoorButton = nullptr;
	// ----
	BrushButton* pzBrushButton = nullptr;
	BrushButton* nopvpBrushButton = nullptr;
	BrushButton* nologBrushButton = nullptr;
	BrushButton* pvpzoneBrushButton = nullptr;

	wxCheckBox* lockDoorCheckbox = nullptr;

	DECLARE_EVENT_TABLE()
};

#endif
