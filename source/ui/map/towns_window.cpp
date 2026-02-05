#include "ui/map/towns_window.h"

#include "editor/editor.h"
#include "map/map.h"
#include "game/town.h"
#include "ui/positionctrl.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"

#include <algorithm>
#include <iterator>

BEGIN_EVENT_TABLE(EditTownsDialog, wxDialog)
EVT_LISTBOX(EDIT_TOWNS_LISTBOX, EditTownsDialog::OnListBoxChange)
EVT_BUTTON(EDIT_TOWNS_SELECT_TEMPLE, EditTownsDialog::OnClickSelectTemplePosition)
EVT_BUTTON(EDIT_TOWNS_ADD, EditTownsDialog::OnClickAdd)
EVT_BUTTON(EDIT_TOWNS_REMOVE, EditTownsDialog::OnClickRemove)
EVT_BUTTON(wxID_OK, EditTownsDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, EditTownsDialog::OnClickCancel)
END_EVENT_TABLE()

EditTownsDialog::EditTownsDialog(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, "Towns", wxDefaultPosition, wxSize(280, 330)),
	editor(editor) {
	Map& map = editor.map;

	// Create topsizer
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	for (const auto& [id, town] : map.towns) {
		town_list.push_back(std::make_unique<Town>(*town));
		if (max_town_id < town->getID()) {
			max_town_id = town->getID();
		}
	}

	// Town list
	town_listbox = newd wxListBox(this, EDIT_TOWNS_LISTBOX, wxDefaultPosition, wxSize(240, 100));
	sizer->Add(town_listbox, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	auto addBtn = newd wxButton(this, EDIT_TOWNS_ADD, "Add");
	addBtn->SetToolTip("Add a new town");
	tmpsizer->Add(addBtn, 0, wxTOP, 5);
	remove_button = newd wxButton(this, EDIT_TOWNS_REMOVE, "Remove");
	remove_button->SetToolTip("Remove selected town");
	tmpsizer->Add(remove_button, 0, wxRIGHT | wxTOP, 5);
	sizer->Add(tmpsizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	// House options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, "Name / ID");
	name_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(190, 20), 0, wxTextValidator(wxFILTER_ASCII, &town_name));
	name_field->SetToolTip("Town name");
	tmpsizer->Add(name_field, 2, wxEXPAND | wxLEFT | wxBOTTOM, 5);

	id_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(40, 20), 0, wxTextValidator(wxFILTER_NUMERIC, &town_id));
	id_field->SetToolTip("Town ID");
	id_field->Enable(false);
	tmpsizer->Add(id_field, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer->Add(tmpsizer, 0, wxEXPAND | wxALL, 10);

	// Temple position
	temple_position = newd PositionCtrl(this, "Temple Position", 0, 0, 0, map.getWidth(), map.getHeight());
	select_position_button = newd wxButton(this, EDIT_TOWNS_SELECT_TEMPLE, "Go To");
	select_position_button->SetToolTip("Jump to temple position");
	temple_position->Add(select_position_button, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer->Add(temple_position, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	auto okBtn = newd wxButton(this, wxID_OK, "OK");
	okBtn->SetToolTip("Save changes");
	tmpsizer->Add(okBtn, wxSizerFlags(1).Center());
	auto cancelBtn = newd wxButton(this, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Cancel");
	tmpsizer->Add(cancelBtn, wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxCENTER | wxALL, 10);

	SetSizerAndFit(sizer);
	Centre(wxBOTH);
	BuildListBox(true);
}

EditTownsDialog::~EditTownsDialog() = default;

void EditTownsDialog::BuildListBox(bool doselect) {
	long tmplong = 0;
	max_town_id = 0;
	wxArrayString town_name_list;
	uint32_t selection_before = 0;

	if (doselect && id_field->GetValue().ToLong(&tmplong)) {
		uint32_t old_town_id = tmplong;

		for (const auto& town : town_list) {
			if (old_town_id == town->getID()) {
				selection_before = town->getID();
				break;
			}
		}
	}

	for (const auto& town : town_list) {
		town_name_list.Add(wxstr(town->getName()));
		if (max_town_id < town->getID()) {
			max_town_id = town->getID();
		}
	}

	town_listbox->Set(town_name_list);
	remove_button->Enable(town_listbox->GetCount() != 0);
	select_position_button->Enable(false);

	if (doselect) {
		if (selection_before) {
			int i = 0;
			for (const auto& town : town_list) {
				if (selection_before == town->getID()) {
					town_listbox->SetSelection(i);
					return;
				}
				++i;
			}
		}
		UpdateSelection(0);
	}
}

void EditTownsDialog::UpdateSelection(int new_selection) {
	long tmplong;

	// Save old values
	if (!town_list.empty()) {
		if (id_field->GetValue().ToLong(&tmplong)) {
			uint32_t old_town_id = tmplong;

			Town* old_town = nullptr;

			for (auto& town : town_list) {
				if (old_town_id == town->getID()) {
					old_town = town.get();
					break;
				}
			}

			if (old_town) {
				editor.map.getOrCreateTile(old_town->getTemplePosition())->getLocation()->decreaseTownCount();

				Position templePos = temple_position->GetPosition();

				editor.map.getOrCreateTile(templePos)->getLocation()->increaseTownCount();

				old_town->setTemplePosition(templePos);

				wxString new_name = name_field->GetValue();
				wxString old_name = wxstr(old_town->getName());

				old_town->setName(nstr(new_name));
				if (new_name != old_name) {
					// Name has changed, update list
					BuildListBox(false);
				}
			}
		}
	}

	// Clear fields
	town_name.Clear();
	town_id.Clear();

	if (town_list.size() > size_t(new_selection)) {
		name_field->Enable(true);
		temple_position->Enable(true);
		select_position_button->Enable(true);

		// Change the values to reflect the newd selection
		Town* town = town_list[new_selection].get();
		ASSERT(town);

		town_name << wxstr(town->getName());
		name_field->SetValue(town_name);
		town_id << long(town->getID());
		id_field->SetValue(town_id);
		temple_position->SetPosition(town->getTemplePosition());
		town_listbox->SetSelection(new_selection);
	} else {
		name_field->Enable(false);
		temple_position->Enable(false);
		select_position_button->Enable(false);
	}
	Refresh();
}

void EditTownsDialog::OnListBoxChange(wxCommandEvent& event) {
	UpdateSelection(event.GetSelection());
}

void EditTownsDialog::OnClickSelectTemplePosition(wxCommandEvent& WXUNUSED(event)) {
	Position templepos = temple_position->GetPosition();
	g_gui.SetScreenCenterPosition(templepos);
}

void EditTownsDialog::OnClickAdd(wxCommandEvent& WXUNUSED(event)) {
	auto new_town = std::make_unique<Town>(++max_town_id);
	new_town->setName("Unnamed Town");
	new_town->setTemplePosition(Position(0, 0, 0));
	town_list.push_back(std::move(new_town));

	editor.map.getOrCreateTile(Position(0, 0, 0))->getLocation()->increaseTownCount();

	BuildListBox(false);
	UpdateSelection(town_list.size() - 1);
	town_listbox->SetSelection(town_list.size() - 1);
}

void EditTownsDialog::OnClickRemove(wxCommandEvent& WXUNUSED(event)) {
	long tmplong;
	if (id_field->GetValue().ToLong(&tmplong)) {
		uint32_t old_town_id = tmplong;

		Town* town = nullptr;
		int selection_index = 0;

		auto town_iter = std::ranges::find_if(town_list, [old_town_id](const std::unique_ptr<Town>& t) {
			return t->getID() == old_town_id;
		});

		if (town_iter != town_list.end()) {
			town = town_iter->get();
			selection_index = static_cast<int>(std::distance(town_list.begin(), town_iter));
		}

		if (!town) {
			return;
		}

		Map& map = editor.map;
		for (const auto& [id, house] : map.houses) {
			if (house->townid == town->getID()) {
				DialogUtil::PopupDialog(this, "Error", "You cannot delete a town which still has houses associated with it.", wxOK);
				return;
			}
		}

		// remove town flag from tile
		editor.map.getOrCreateTile(town->getTemplePosition())->getLocation()->decreaseTownCount();

		// remove town object
		town_list.erase(town_iter);
		BuildListBox(false);
		UpdateSelection(selection_index - 1);
	}
}

void EditTownsDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	long tmplong = 0;

	if (Validate() && TransferDataFromWindow()) {
		// Save old values
		if (!town_list.empty() && id_field->GetValue().ToLong(&tmplong)) {
			uint32_t old_town_id = tmplong;

			Town* old_town = nullptr;

			for (auto& town : town_list) {
				if (old_town_id == town->getID()) {
					old_town = town.get();
					break;
				}
			}

			if (old_town) {
				editor.map.getOrCreateTile(old_town->getTemplePosition())->getLocation()->decreaseTownCount();

				Position templePos = temple_position->GetPosition();

				editor.map.getOrCreateTile(templePos)->getLocation()->increaseTownCount();

				old_town->setTemplePosition(templePos);

				wxString new_name = name_field->GetValue();
				wxString old_name = wxstr(old_town->getName());

				old_town->setName(nstr(new_name));
				if (new_name != old_name) {
					// Name has changed, update list
					BuildListBox(true);
				}
			}
		}

		Towns& towns = editor.map.towns;

		// Verify the newd information
		for (const auto& town : town_list) {
			if (town->getName() == "") {
				DialogUtil::PopupDialog(this, "Error", "You can't have a town with an empty name.", wxOK);
				return;
			}
			if (!town->getTemplePosition().isValid() || town->getTemplePosition().x > editor.map.getWidth() || town->getTemplePosition().y > editor.map.getHeight()) {
				wxString msg;
				msg << "The town " << wxstr(town->getName()) << " has an invalid temple position.";
				DialogUtil::PopupDialog(this, "Error", msg, wxOK);
				return;
			}
		}

		// Clear old towns
		towns.clear();

		// Build the newd town map
		for (auto& town : town_list) {
			// Movement to towns.addTown() leaves moved-from unique_ptrs in town_list.
			// This is safe as town_list.clear() is called immediately after.
			towns.addTown(std::move(town));
		}
		town_list.clear();
		editor.map.doChange();

		EndModal(1);
		g_gui.RefreshPalettes();
	}
}

void EditTownsDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}
