//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_PALETTE_COMMONS_H_
#define RME_PALETTE_COMMONS_H_

#include "app/main.h"

#include "ui/dcbutton.h"
#include "map/tileset.h"
#include "ui/gui_ids.h"
#include "brushes/brush_enums.h"
#include "ui/controls/item_buttons.h"

class GUI;
class BrushButton;
class TilesetCategory;
class BrushSizePanel;
class BrushPalettePanel;
class PaletteWindow;

using PaletteType = TilesetCategoryType;

class PalettePanel : public wxPanel {
public:
	PalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY, long style = wxTAB_TRAVERSAL);
	~PalettePanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	virtual void InvalidateContents();
	// Loads the currently displayed page
	virtual void LoadCurrentContents();
	// Loads all content in this panel
	virtual void LoadAllContents();

	PaletteWindow* GetParentPalette() const;
	virtual wxString GetName() const;
	virtual PaletteType GetType() const;

	// Add a tool panel!
	virtual void AddToolPanel(PalettePanel* panel);
	// Sets the style for this toolbar and child toolabrs
	virtual void SetToolbarIconSize(bool large_icons);

	// Select the first brush
	virtual void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	virtual Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	virtual int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	virtual bool SelectBrush(const Brush* whatbrush);

	// Updates the palette window to use the current brush size
	virtual void OnUpdateBrushSize(BrushShape shape, int size);
	// Called when this page is about to be displayed
	virtual void OnSwitchIn();
	// Called when this page is hidden
	virtual void OnSwitchOut();
	// Called sometimes
	virtual void OnUpdate();
	// When the palette should do a delayed refresh (necessary for multiple palettes)
	void OnRefreshTimer(wxTimerEvent&);

	void RefreshOtherPalettes();

protected:
	using ToolBarList = std::vector<PalettePanel*>;
	ToolBarList tool_bars;
	wxTimer refresh_timer;
	int last_brush_size;

	DECLARE_EVENT_TABLE();
};

#endif
