//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "map/tile.h"
#include "game/item.h"
#include "game/complexitem.h"
#include "game/town.h"
#include "game/house.h"
#include "map/map.h"
#include "editor/editor.h"
#include "game/creature.h"

#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "app/application.h"
#include "ui/old_properties_window.h"

// ============================================================================
// Old Properties Window

BEGIN_EVENT_TABLE(OldPropertiesWindow, wxDialog)
EVT_SET_FOCUS(OldPropertiesWindow::OnFocusChange)
EVT_BUTTON(wxID_OK, OldPropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, OldPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Item Properties", map, tile_parent, item, pos),
	count_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr) {
	ASSERT(edit_item);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	// Normal item
	Door* door = dynamic_cast<Door*>(edit_item);
	Teleport* teleport = dynamic_cast<Teleport*>(edit_item);

	wxString description;
	if (door) {
		ASSERT(tile_parent);
		description = "Door Properties";
	} else if (teleport) {
		description = "Teleport Properties";
	} else {
		description = "Item Properties";
	}

	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, description);

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

	subsizer->Add(newd wxStaticText(this, wxID_ANY, (item->isCharged() ? "Charges" : "Count")));
	int max_count = 100;
	if (item->isClientCharged()) {
		max_count = 250;
	}
	if (item->isExtraCharged()) {
		max_count = 65500;
	}
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getCount()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, max_count, edit_item->getCount());
	if (!item->isStackable() && !item->isCharged()) {
		count_field->Enable(false);
	}
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
	action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
	unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

	// item classification (12.81+)
	if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Classification"));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(item->getClassification())));

		// item iter
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tier"));
		tier_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
		subsizer->Add(tier_field, wxSizerFlags(1).Expand());
	}

	if (door) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Door ID"));
		door_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(door->getDoorID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, door->getDoorID());
		if (!edit_tile || !edit_tile->isHouseTile() || !door->isRealDoor()) {
			door_id_field->Disable();
		}
		subsizer->Add(door_id_field, wxSizerFlags(1).Expand());
	}

	if (teleport) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Destination"));

		wxSizer* possizer = newd wxBoxSizer(wxHORIZONTAL);
		x_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getX()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getWidth(), teleport->getX());
		x_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(x_field, wxSizerFlags(3).Expand());
		y_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getY()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getHeight(), teleport->getY());
		y_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(y_field, wxSizerFlags(3).Expand());
		z_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getZ()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, MAP_MAX_LAYER, teleport->getZ());
		z_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(z_field, wxSizerFlags(2).Expand());

		subsizer->Add(possizer, wxSizerFlags(1).Expand());
	}

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));

	wxSizer* subsizer_btn = newd wxBoxSizer(wxHORIZONTAL);
	subsizer_btn->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	subsizer_btn->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(subsizer_btn, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::~OldPropertiesWindow() {
	//
}

void OldPropertiesWindow::OnFocusChange(wxFocusEvent& event) {
	wxWindow* win = event.GetWindow();
	if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win)) {
		spin->SetSelection(-1, -1);
	} else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win)) {
		text->SetSelection(-1, -1);
	}
}

void OldPropertiesWindow::OnChar(wxKeyEvent& evt) {
	if (evt.GetKeyCode() == WXK_CONTROL_V) {
		Position position;
		const Editor* const editor = g_gui.GetCurrentEditor();
		if (posFromClipboard(position, editor->map.getWidth(), editor->map.getHeight())) {
			x_field->SetValue(position.x);
			y_field->SetValue(position.y);
			z_field->SetValue(position.z);
			return;
		}
	}

	evt.Skip();
}

void OldPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (edit_item) {
		// Normal item
		Door* door = dynamic_cast<Door*>(edit_item);
		Teleport* teleport = dynamic_cast<Teleport*>(edit_item);

		int new_uid = unique_id_field->GetValue();
		int new_aid = action_id_field->GetValue();
		int new_count = count_field ? count_field->GetValue() : 1;
		int new_tier = tier_field ? tier_field->GetValue() : 0;

		Position new_dest;
		if (teleport) {
			new_dest = Position(x_field->GetValue(), y_field->GetValue(), z_field->GetValue());
		}
		uint8_t new_door_id = 0;
		if (door) {
			new_door_id = door_id_field->GetValue();
		}

		if ((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
			DialogUtil::PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
			return;
		}
		if ((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
			DialogUtil::PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
			return;
		}
		if (new_tier < 0 || new_tier > 0xFF) {
			DialogUtil::PopupDialog(this, "Error", "Item tier must be between 0 and 255.", wxOK);
			return;
		}

		if (door && g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
			if (edit_tile && edit_tile->isHouseTile()) {
				const House* house = edit_map->houses.getHouse(edit_tile->getHouseID());
				if (house) {
					Position pos = house->getDoorPositionByID(new_door_id);
					if (pos == Position()) {
						// Do nothing
					} else if (pos != edit_tile->getPosition()) {
						int ret = DialogUtil::PopupDialog(this, "Warning", "This doorid conflicts with another one in this house, are you sure you want to continue?", wxYES | wxNO);
						if (ret == wxID_NO) {
							return;
						}
					}
				}
			}
		}

		if (teleport) {
			if (edit_map->getTile(new_dest) == nullptr || edit_map->getTile(new_dest)->isBlocking()) {
				int ret = DialogUtil::PopupDialog(this, "Warning", "This teleport leads nowhere, or to an invalid location. Do you want to change the destination?", wxYES | wxNO);
				if (ret == wxID_YES) {
					return;
				}
			}
		}

		// Done validating, set the values.
		if (edit_item->isStackable() || edit_item->isCharged()) {
			edit_item->setSubtype(new_count);
		}
		if (door) {
			door->setDoorID(new_door_id);
		}
		if (teleport) {
			teleport->setDestination(new_dest);
		}
		edit_item->setUniqueID(new_uid);
		edit_item->setActionID(new_aid);
		edit_item->setTier(new_tier);
	}
	EndModal(1);
}

void OldPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
