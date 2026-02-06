//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_HOUSE_PALETTE_H_
#define RME_HOUSE_PALETTE_H_

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/dataview.h>
#include <wx/bitmap.h>
#include "brushes/brush.h"
#include <wx/tglbtn.h>
#include <wx/timer.h>

class Map;
class House;
class Brush;

class HousePalette : public wxPanel {
public:
	HousePalette(wxWindow* parent);
	virtual ~HousePalette();

	void SetMap(Map* map);
	void UpdateHouses();

	// Palette operations
	Brush* GetSelectedBrush() const;
	void SelectHouseBrush();
	void SelectExitBrush();

	// Event handlers
	void OnTownChange(wxCommandEvent& event);
	void OnSearchChange(wxCommandEvent& event);
	void OnHouseSelected(wxDataViewEvent& event);
	void OnHouseActivated(wxDataViewEvent& event);

	void OnAddHouse(wxCommandEvent& event);
	void OnEditHouse(wxCommandEvent& event);
	void OnRemoveHouse(wxCommandEvent& event);

	void OnHouseBrushButton(wxCommandEvent& event);
	void OnSelectExitButton(wxCommandEvent& event);

	void OnSearchCharHook(wxKeyEvent& event);

protected:
	void FilterHouses();
	House* GetSelectedHouse() const;

	Map* map;

	wxChoice* town_choice;
	wxTextCtrl* search_ctrl;
	wxDataViewListCtrl* house_list;

	wxToggleButton* house_brush_button;
	wxToggleButton* select_exit_button;

	wxButton* add_button;
	wxButton* edit_button;
	wxButton* remove_button;

	wxBitmap guild_icon;
	wxBitmap no_guild_icon;
};

#endif
