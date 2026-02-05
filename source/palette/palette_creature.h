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

#ifndef RME_PALETTE_CREATURE_H_
#define RME_PALETTE_CREATURE_H_

#include "brushes/brush_enums.h"
#include "palette/palette_common.h"
#include "palette/panels/brush_panel.h"
#include "ui/controls/sortable_list_box.h"

class CreaturePalettePanel : public PalettePanel {
public:
	CreaturePalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~CreaturePalettePanel() override = default;

	PaletteType GetType() const override;

	// Select the first brush
	void SelectFirstBrush() override;
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const override;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const override;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush) override;

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size) override;
	// Called when this page is displayed
	void OnSwitchIn() override;
	// Called sometimes?
	void OnUpdate() override;
	void OnRefreshTilesets();

	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);

protected:
	void SelectTileset(size_t index);
	void SelectCreature(size_t index);
	void SelectCreature(std::string name);

public:
	// Event handling
	void OnChangeSpawnTime(wxSpinEvent& event);
	void OnChangeSpawnSize(wxSpinEvent& event);

	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
	void OnClickCreatureBrushButton(wxCommandEvent& event);
	void OnClickSpawnBrushButton(wxCommandEvent& event);

protected:
	void SelectCreatureBrush();
	void SelectSpawnBrush();

	wxChoicebook* choicebook;
	wxToggleButton* creature_brush_button;
	wxToggleButton* spawn_brush_button;
	wxSpinCtrl* creature_spawntime_spin;
	wxSpinCtrl* spawn_size_spin;

	bool handling_event;

	DECLARE_EVENT_TABLE();
};

#endif
