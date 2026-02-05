//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "palette/house/house_palette.h"
#include "palette/house/edit_house_dialog.h"

#include "map/map.h"
#include "game/house.h"
#include "game/town.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"

#include "brushes/managers/brush_manager.h"
#include "brushes/house/house_brush.h"
#include "brushes/house/house_exit_brush.h"

#include <algorithm>

enum {
	ID_TOWN_CHOICE = 10000,
	ID_SEARCH_CTRL,
	ID_HOUSE_LIST,
	ID_ADD_HOUSE,
	ID_EDIT_HOUSE,
	ID_REMOVE_HOUSE,
	ID_HOUSE_BRUSH,
	ID_EXIT_BRUSH
};

BEGIN_EVENT_TABLE(HousePalette, wxPanel)
EVT_CHOICE(ID_TOWN_CHOICE, HousePalette::OnTownChange)
EVT_TEXT(ID_SEARCH_CTRL, HousePalette::OnSearchChange)
EVT_DATAVIEW_SELECTION_CHANGED(ID_HOUSE_LIST, HousePalette::OnHouseSelected)
EVT_DATAVIEW_ITEM_ACTIVATED(ID_HOUSE_LIST, HousePalette::OnHouseActivated)

EVT_BUTTON(ID_ADD_HOUSE, HousePalette::OnAddHouse)
EVT_BUTTON(ID_EDIT_HOUSE, HousePalette::OnEditHouse)
EVT_BUTTON(ID_REMOVE_HOUSE, HousePalette::OnRemoveHouse)

EVT_TOGGLEBUTTON(ID_HOUSE_BRUSH, HousePalette::OnHouseBrushButton)
EVT_TOGGLEBUTTON(ID_EXIT_BRUSH, HousePalette::OnSelectExitButton)
END_EVENT_TABLE()

HousePalette::HousePalette(wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	map(nullptr) {

	wxBoxSizer* main_sizer = newd wxBoxSizer(wxVERTICAL);

	// Header section (Town & Search)
	wxStaticBoxSizer* filter_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Filter / Search");

	town_choice = newd wxChoice(this, ID_TOWN_CHOICE);
	filter_sizer->Add(town_choice, 0, wxEXPAND | wxALL, 2);

	search_ctrl = newd wxTextCtrl(this, ID_SEARCH_CTRL, "", wxDefaultPosition, wxDefaultSize, 0);
	search_ctrl->SetHint("Search houses...");
	filter_sizer->Add(search_ctrl, 0, wxEXPAND | wxALL, 2);

	main_sizer->Add(filter_sizer, 0, wxEXPAND | wxALL, 5);

	// Table section
	house_list = newd wxDataViewListCtrl(this, ID_HOUSE_LIST, wxDefaultPosition, wxDefaultSize, wxDV_SINGLE | wxDV_ROW_LINES);
	house_list->AppendIconTextColumn("Guild", wxDATAVIEW_CELL_INERT, 40, wxALIGN_CENTER, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);
	house_list->AppendTextColumn("ID", wxDATAVIEW_CELL_INERT, 40, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);
	house_list->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);
	house_list->AppendTextColumn("Rent", wxDATAVIEW_CELL_INERT, 60, wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);

	main_sizer->Add(house_list, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// Action buttons (Add, Edit, Remove)
	wxBoxSizer* action_sizer = newd wxBoxSizer(wxHORIZONTAL);
	add_button = newd wxButton(this, ID_ADD_HOUSE, "Add", wxDefaultPosition, wxSize(50, -1));
	edit_button = newd wxButton(this, ID_EDIT_HOUSE, "Edit", wxDefaultPosition, wxSize(50, -1));
	remove_button = newd wxButton(this, ID_REMOVE_HOUSE, "Remove", wxDefaultPosition, wxSize(60, -1));

	action_sizer->Add(add_button, 1, wxEXPAND | wxRIGHT, 2);
	action_sizer->Add(edit_button, 1, wxEXPAND | wxRIGHT, 2);
	action_sizer->Add(remove_button, 1, wxEXPAND);

	main_sizer->Add(action_sizer, 0, wxEXPAND | wxALL, 5);

	// Brush buttons (House tiles, Select Exit)
	wxStaticBoxSizer* brush_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Brushes");

	house_brush_button = newd wxToggleButton(this, ID_HOUSE_BRUSH, "House tiles");
	select_exit_button = newd wxToggleButton(this, ID_EXIT_BRUSH, "Select Exit");

	brush_sizer->Add(house_brush_button, 0, wxEXPAND | wxBOTTOM, 2);
	brush_sizer->Add(select_exit_button, 0, wxEXPAND);

	main_sizer->Add(brush_sizer, 0, wxEXPAND | wxALL, 5);

	SetSizer(main_sizer);

	// Initialize icons
	guild_icon = wxBitmap(16, 16);
	{
		wxMemoryDC dc;
		dc.SelectObject(guild_icon);
		dc.SetBackground(*wxTRANSPARENT_BRUSH);
		dc.Clear();
		dc.SetPen(*wxGREEN_PEN);
		dc.SetBrush(*wxGREEN_BRUSH);
		dc.DrawCircle(8, 8, 6);
		dc.SelectObject(wxNullBitmap);
	}
	no_guild_icon = wxBitmap(16, 16);
	{
		wxMemoryDC dc;
		dc.SelectObject(no_guild_icon);
		dc.SetBackground(*wxTRANSPARENT_BRUSH);
		dc.Clear();
		dc.SelectObject(wxNullBitmap);
	}

	search_ctrl->Bind(wxEVT_CHAR_HOOK, &HousePalette::OnSearchCharHook, this);
}

HousePalette::~HousePalette() {
}

void HousePalette::SetMap(Map* m) {
	if (map != m) {
		g_brush_manager.house_brush->setHouse(nullptr);
	}
	map = m;
	UpdateHouses();
}

void HousePalette::UpdateHouses() {
	town_choice->Clear();
	house_list->DeleteAllItems();

	if (!map) {
		return;
	}

	// Populate towns
	town_choice->Append("All Towns", (void*)nullptr);
	for (auto& it : map->towns) {
		town_choice->Append(wxstr(it.second->getName()), it.second.get());
	}
	town_choice->Append("No Town", (void*)nullptr);
	town_choice->SetSelection(0);

	FilterHouses();
}

void HousePalette::FilterHouses() {
	house_list->DeleteAllItems();
	if (!map) {
		return;
	}

	wxString search_query = search_ctrl->GetValue().Lower();
	int town_idx = town_choice->GetSelection();
	Town* selected_town = nullptr;
	bool filter_no_town = false;

	if (town_idx != wxNOT_FOUND) {
		if (town_idx > 0 && town_idx < (int)town_choice->GetCount() - 1) {
			selected_town = (Town*)town_choice->GetClientData(town_idx);
		} else if (town_idx == (int)town_choice->GetCount() - 1) {
			filter_no_town = true;
		}
	}

	for (auto& it : map->houses) {
		House* house = it.second.get();

		bool search_match = search_query.IsEmpty() || wxstr(house->name).Lower().Contains(search_query) || wxstr(std::to_string(house->getID())).Contains(search_query);

		// If searching across all cities, we ignore town filter if search is not empty
		bool town_match = true;
		if (search_query.IsEmpty()) {
			if (selected_town) {
				town_match = (house->townid == selected_town->getID());
			} else if (filter_no_town) {
				town_match = (map->towns.getTown(house->townid) == nullptr);
			}
		}

		if (search_match && town_match) {
			wxVector<wxVariant> data;
			wxIcon icon;
			icon.CopyFromBitmap(house->guildhall ? guild_icon : no_guild_icon);
			data.push_back(wxVariant(wxDataViewIconText("", icon)));
			data.push_back(wxVariant(wxstr(std::to_string(house->getID()))));
			data.push_back(wxVariant(wxstr(house->name)));
			data.push_back(wxVariant(wxstr(std::to_string(house->rent))));

			// Store pointer to house as client data if possible?
			// wxDataViewListCtrl doesn't directly store client data per row easily like wxListBox.
			// We can use the ID to find it.
			house_list->AppendItem(data, (wxUIntPtr)house);
		}
	}
}

House* HousePalette::GetSelectedHouse() const {
	wxDataViewItem item = house_list->GetSelection();
	if (item.IsOk()) {
		return (House*)house_list->GetItemData(item);
	}
	return nullptr;
}

Brush* HousePalette::GetSelectedBrush() const {
	House* house = GetSelectedHouse();
	if (select_exit_button->GetValue()) {
		if (house) {
			g_brush_manager.house_exit_brush->setHouse(house);
		}
		return (g_brush_manager.house_exit_brush->getHouseID() != 0 ? g_brush_manager.house_exit_brush : nullptr);
	} else if (house_brush_button->GetValue()) {
		g_brush_manager.house_brush->setHouse(house);
		return (g_brush_manager.house_brush->getHouseID() != 0 ? g_brush_manager.house_brush : nullptr);
	}
	return nullptr;
}

void HousePalette::SelectHouseBrush() {
	house_brush_button->SetValue(true);
	select_exit_button->SetValue(false);
	g_gui.SelectBrush();
}

void HousePalette::SelectExitBrush() {
	house_brush_button->SetValue(false);
	select_exit_button->SetValue(true);
	g_gui.SelectBrush();
}

void HousePalette::OnTownChange(wxCommandEvent& event) {
	FilterHouses();
}

void HousePalette::OnSearchChange(wxCommandEvent& event) {
	FilterHouses();
}

void HousePalette::OnHouseSelected(wxDataViewEvent& event) {
	House* house = GetSelectedHouse();
	if (house) {
		if (house_brush_button->GetValue()) {
			g_brush_manager.house_brush->setHouse(house);
		} else if (select_exit_button->GetValue()) {
			g_brush_manager.house_exit_brush->setHouse(house);
		}
		g_gui.SelectBrush();
	}
}

void HousePalette::OnHouseActivated(wxDataViewEvent& event) {
	House* house = GetSelectedHouse();
	if (house) {
		SelectHouseBrush();
		g_gui.SelectBrush(); // Ensure brush is activated
		if (house->getExit() != Position(0, 0, 0) && house->getExit().isValid()) {
			g_gui.SetScreenCenterPosition(house->getExit());
		}
	}
}

void HousePalette::OnSearchCharHook(wxKeyEvent& event) {
	event.Skip();
}

void HousePalette::OnAddHouse(wxCommandEvent& event) {
	if (!map) {
		return;
	}

	auto new_house = std::make_unique<House>(*map);
	new_house->setID(map->houses.getEmptyID());

	std::ostringstream os;
	os << "Unnamed House #" << new_house->getID();
	new_house->name = os.str();

	// Assign to selected town if any
	int town_idx = town_choice->GetSelection();
	if (town_idx > 0 && town_idx < (int)town_choice->GetCount() - 1) {
		Town* town = (Town*)town_choice->GetClientData(town_idx);
		new_house->townid = town->getID();
	}

	House* house_ptr = new_house.get();
	map->houses.addHouse(std::move(new_house));
	FilterHouses();

	// Select the new house
	for (int i = 0; i < (int)house_list->GetItemCount(); ++i) {
		wxDataViewItem item = house_list->RowToItem(i);
		if ((House*)house_list->GetItemData(item) == house_ptr) {
			house_list->Select(item);
			break;
		}
	}

	g_gui.SelectBrush();
}

void HousePalette::OnEditHouse(wxCommandEvent& event) {
	House* house = GetSelectedHouse();
	if (house && map) {
		EditHouseDialog* d = newd EditHouseDialog(g_gui.root, map, house);
		if (d->ShowModal() == 1) {
			FilterHouses();
			g_gui.SelectBrush();
		}
	}
}

void HousePalette::OnRemoveHouse(wxCommandEvent& event) {
	House* house = GetSelectedHouse();
	if (house && map) {
		int ret = wxMessageBox("Are you sure you want to remove this house? This cannot be undone.", "Remove House", wxYES_NO | wxICON_WARNING | wxCENTER, this);
		if (ret == wxYES) {
			map->houses.removeHouse(house);
			FilterHouses();
			g_gui.SelectBrush();
			g_gui.RefreshView();
		}
	}
}

void HousePalette::OnHouseBrushButton(wxCommandEvent& event) {
	if (house_brush_button->GetValue()) {
		select_exit_button->SetValue(false);
	}
	g_gui.SelectBrush();
}

void HousePalette::OnSelectExitButton(wxCommandEvent& event) {
	if (select_exit_button->GetValue()) {
		house_brush_button->SetValue(false);
	}
	g_gui.SelectBrush();
}
