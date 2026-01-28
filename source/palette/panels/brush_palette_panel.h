#ifndef RME_PALETTE_PANELS_BRUSH_PALETTE_PANEL_H_
#define RME_PALETTE_PANELS_BRUSH_PALETTE_PANEL_H_

#include "palette/palette_common.h"
#include "palette/panels/brush_panel.h"
#include "app/settings.h"

class BrushPalettePanel : public PalettePanel {
public:
	BrushPalettePanel(wxWindow* parent, const TilesetContainer& tilesets, TilesetCategoryType category, wxWindowID id = wxID_ANY);
	~BrushPalettePanel() override;

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents() override;
	// Loads the currently displayed page
	void LoadCurrentContents() override;
	// Loads all content in this panel
	void LoadAllContents() override;

	PaletteType GetType() const override;

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);

	// Select the first brush
	void SelectFirstBrush() override;
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const override;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush) override;

	// Called when this page is displayed
	void OnSwitchIn() override;

	// Event handler for child window
	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
	void OnClickAddTileset(wxCommandEvent& WXUNUSED(event));
	void OnClickAddItemToTileset(wxCommandEvent& WXUNUSED(event));

protected:
	PaletteType palette_type;
	wxChoicebook* choicebook;

	// No size_panel, it was unused

	std::map<wxWindow*, Brush*> remembered_brushes;

	DECLARE_EVENT_TABLE();
};

#endif
