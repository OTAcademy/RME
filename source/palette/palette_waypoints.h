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

#ifndef RME_PALETTE_WAYPOINTS_H_
#define RME_PALETTE_WAYPOINTS_H_

#include <wx/listctrl.h>

#include "game/waypoints.h"
#include "palette/palette_common.h"

class WaypointPalettePanel : public PalettePanel {
public:
	WaypointPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~WaypointPalettePanel() override = default;

	wxString GetName() const override;
	PaletteType GetType() const override;

	// Select the first brush
	void SelectFirstBrush() override;
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const override;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const override;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush) override;

	// Called sometimes?
	void OnUpdate() override;
	// Called when this page is about to be displayed
	void OnSwitchIn() override;
	// Called when this page is hidden
	void OnSwitchOut() override;

public:
	// wxWidgets event handling
	void OnClickWaypoint(wxListEvent& event);
	void OnBeginEditWaypointLabel(wxListEvent& event);
	void OnEditWaypointLabel(wxListEvent& event);
	void OnClickAddWaypoint(wxCommandEvent& event);
	void OnClickRemoveWaypoint(wxCommandEvent& event);

	void SetMap(Map* map);

protected:
	Map* map;
	wxListCtrl* waypoint_list;
	wxButton* add_waypoint_button;
	wxButton* remove_waypoint_button;

	DECLARE_EVENT_TABLE()
};

#endif
