//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "palette/house/edit_house_dialog.h"

#include "ui/dialog_util.h"
#include "app/settings.h"
#include "ui/gui.h"

#include "map/map.h"
#include "game/house.h"
#include "game/town.h"

#include <sstream>

BEGIN_EVENT_TABLE(EditHouseDialog, wxDialog)
EVT_SET_FOCUS(EditHouseDialog::OnFocusChange)
EVT_BUTTON(wxID_OK, EditHouseDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, EditHouseDialog::OnClickCancel)
END_EVENT_TABLE()

EditHouseDialog::EditHouseDialog(wxWindow* parent, Map* map, House* house) :
	wxDialog(parent, wxID_ANY, "House Properties", wxDefaultPosition, wxSize(250, 160)),
	map(map),
	what_house(house) {
	ASSERT(map);
	ASSERT(house);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "House Properties");
	wxFlexGridSizer* housePropContainer = newd wxFlexGridSizer(2, 10, 10);
	housePropContainer->AddGrowableCol(1);

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	house_name = wxstr(house->name);
	house_id = i2ws(house->getID());
	house_rent = i2ws(house->rent);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Name:"), wxSizerFlags(0).Border(wxLEFT, 5));
	name_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_ASCII, &house_name));
	subsizer->Add(name_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Town:"), wxSizerFlags(0).Border(wxLEFT, 5));

	const Towns& towns = map->towns;
	town_id_field = newd wxChoice(this, wxID_ANY);
	int to_select_index = 0;
	uint32_t houseTownId = house->townid;

	if (towns.count() > 0) {
		bool found = false;
		for (TownMap::const_iterator town_iter = towns.begin(); town_iter != towns.end(); ++town_iter) {
			if (town_iter->second->getID() == houseTownId) {
				found = true;
			}
			town_id_field->Append(wxstr(town_iter->second->getName()));
			town_ids_.push_back(town_iter->second->getID());
			if (!found) {
				++to_select_index;
			}
		}

		if (!found) {
			if (houseTownId != 0) {
				town_id_field->Append("Undefined Town (id:" + i2ws(houseTownId) + ")");
				town_ids_.push_back(houseTownId);
				++to_select_index;
			}
		}
	}
	town_id_field->SetSelection(to_select_index);
	subsizer->Add(town_id_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Rent:"), wxSizerFlags(0).Border(wxLEFT, 5));
	rent_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_NUMERIC, &house_rent));
	subsizer->Add(rent_field, wxSizerFlags(1).Expand());

	wxFlexGridSizer* subsizerRight = newd wxFlexGridSizer(1, 10, 10);
	wxFlexGridSizer* houseSizer = newd wxFlexGridSizer(2, 10, 10);

	houseSizer->Add(newd wxStaticText(this, wxID_ANY, "ID:"), wxSizerFlags(0).Center());
	id_field = newd wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(40, 20), wxSP_ARROW_KEYS, 1, 0xFFFF, house->getID());
	houseSizer->Add(id_field, wxSizerFlags(1).Expand());
	subsizerRight->Add(houseSizer, wxSizerFlags(1).Expand());

	wxSizer* checkbox_sub_sizer = newd wxBoxSizer(wxVERTICAL);
	checkbox_sub_sizer->AddSpacer(4);
	guildhall_field = newd wxCheckBox(this, wxID_ANY, "Guildhall");
	checkbox_sub_sizer->Add(guildhall_field);
	subsizerRight->Add(checkbox_sub_sizer);
	guildhall_field->SetValue(house->guildhall);

	housePropContainer->Add(subsizer, wxSizerFlags(5).Expand());
	housePropContainer->Add(subsizerRight, wxSizerFlags(5).Expand());
	boxsizer->Add(housePropContainer, wxSizerFlags(5).Expand().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxRIGHT | wxLEFT, 20));

	wxSizer* buttonsSizer = newd wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	buttonsSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(buttonsSizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
}

EditHouseDialog::~EditHouseDialog() {
}

void EditHouseDialog::OnFocusChange(wxFocusEvent& event) {
	wxWindow* win = event.GetWindow();
	if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win)) {
		spin->SetSelection(-1, -1);
	} else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win)) {
		text->SetSelection(-1, -1);
	}
}

void EditHouseDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (Validate() && TransferDataFromWindow()) {
		long new_house_rent;
		house_rent.ToLong(&new_house_rent);
		if (new_house_rent < 0) {
			DialogUtil::PopupDialog(this, "Error", "House rent cannot be less than 0.", wxOK);
			return;
		}

		uint32_t new_house_id = id_field->GetValue();
		if (new_house_id < 1) {
			DialogUtil::PopupDialog(this, "Error", "House id cannot be less than 1.", wxOK);
			return;
		}

		if (house_name.length() == 0) {
			DialogUtil::PopupDialog(this, "Error", "House name cannot be empty.", wxOK);
			return;
		}

		if (g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
			Houses& houses = map->houses;
			for (const auto& [house_id, house_ptr] : houses) {
				House* house = house_ptr.get();
				ASSERT(house);

				if (house->getID() == new_house_id && new_house_id != what_house->getID()) {
					DialogUtil::PopupDialog(this, "Error", "This house id is already in use.", wxOK);
					return;
				}

				if (wxstr(house->name) == house_name && house->getID() != what_house->getID()) {
					int ret = DialogUtil::PopupDialog(this, "Warning", "This house name is already in use, are you sure you want to continue?", wxYES | wxNO);
					if (ret == wxID_NO) {
						return;
					}
				}
			}
		}

		if (new_house_id != what_house->getID()) {
			int ret = DialogUtil::PopupDialog(this, "Warning", "Changing existing house ids on a production server WILL HAVE DATABASE CONSEQUENCES such as potential item loss, house owner change or invalidating guest lists.\nYou are doing it at own risk!\n\nAre you ABSOLUTELY sure you want to continue?", wxYES | wxNO);
			if (ret == wxID_NO) {
				return;
			}

			uint32_t old_house_id = what_house->getID();
			map->convertHouseTiles(old_house_id, new_house_id);
			map->houses.changeId(what_house, new_house_id);
		}

		int selection = town_id_field->GetSelection();
		if (selection == wxNOT_FOUND || selection < 0 || (size_t)selection >= town_ids_.size()) {
			DialogUtil::PopupDialog(this, "Error", "Invalid town selected.", wxOK);
			return;
		}
		int new_town_id = town_ids_[selection];

		what_house->name = nstr(house_name);
		what_house->rent = new_house_rent;
		what_house->guildhall = guildhall_field->GetValue();
		what_house->townid = new_town_id;

		EndModal(1);
	}
}

void EditHouseDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
