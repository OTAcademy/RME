#ifndef RME_PALETTE_PANELS_BRUSH_PANEL_H_
#define RME_PALETTE_PANELS_BRUSH_PANEL_H_

#include "app/main.h"
#include "map/tileset.h"
#include "brushes/brush.h"
#include "palette/controls/brush_button.h"

enum BrushListType {
	BRUSHLIST_LARGE_ICONS,
	BRUSHLIST_SMALL_ICONS,
	BRUSHLIST_LISTBOX,
	BRUSHLIST_TEXT_LISTBOX,
};

class BrushBoxInterface {
public:
	BrushBoxInterface(const TilesetCategory* _tileset) :
		tileset(_tileset), loaded(false) {
		ASSERT(tileset);
	}
	virtual ~BrushBoxInterface() { }

	virtual wxWindow* GetSelfWindow() = 0;

	// Select the first brush
	virtual void SelectFirstBrush() = 0;
	// Returns the currently selected brush (First brush if panel is not loaded)
	virtual Brush* GetSelectedBrush() const = 0;
	// Select the brush in the parameter, this only changes the look of the panel
	virtual bool SelectBrush(const Brush* brush) = 0;

protected:
	const TilesetCategory* const tileset;
	bool loaded;
};

class BrushPanel : public wxPanel {
public:
	BrushPanel(wxWindow* parent);
	~BrushPanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents();
	// Loads the content (This must be called before the panel is displayed, else it will appear empty
	void LoadContents();

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);
	// Assigns a tileset to this list
	void AssignTileset(const TilesetCategory* tileset);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called when the window is about to be displayed
	void OnSwitchIn();
	// Called when this page is hidden
	void OnSwitchOut();

protected:
	const TilesetCategory* tileset;
	wxSizer* sizer;
	BrushBoxInterface* brushbox;
	bool loaded;
	BrushListType list_type;
};

#endif
