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
#include "ui/properties/old_properties_window.h"
#include "ui/properties/property_validator.h"
#include "ui/properties/property_applier.h"
#include "ui/properties/teleport_service.h"

// ============================================================================
// Old Properties Window

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Item Properties", map, tile_parent, item, pos),
	count_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr) {
	ASSERT(edit_item);

	Bind(wxEVT_SET_FOCUS, &OldPropertiesWindow::OnFocusChange, this);
	Bind(wxEVT_BUTTON, &OldPropertiesWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &OldPropertiesWindow::OnClickCancel, this, wxID_CANCEL);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxString description;
	if (dynamic_cast<Door*>(edit_item)) {
		ASSERT(tile_parent);
		description = "Door Properties";
	} else if (dynamic_cast<Teleport*>(edit_item)) {
		description = "Teleport Properties";
	} else {
		description = "Item Properties";
	}

	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, description);
	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	createHeaderFields(subsizer);
	createGenericFields(subsizer);
	createClassificationFields(subsizer);
	createDoorFields(subsizer);
	createTeleportFields(subsizer);

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());
	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));

	wxSizer* subsizer_btn = newd wxBoxSizer(wxHORIZONTAL);
	subsizer_btn->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	subsizer_btn->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(subsizer_btn, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

void OldPropertiesWindow::createHeaderFields(wxFlexGridSizer* subsizer) {
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(edit_item->getID())));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));
}

void OldPropertiesWindow::createGenericFields(wxFlexGridSizer* subsizer) {
	subsizer->Add(newd wxStaticText(this, wxID_ANY, (edit_item->isCharged() ? "Charges" : "Count")));
	int max_count = 100;
	if (edit_item->isClientCharged()) {
		max_count = 250;
	}
	if (edit_item->isExtraCharged()) {
		max_count = 65500;
	}
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getCount()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, max_count, edit_item->getCount());
	if (!edit_item->isStackable() && !edit_item->isCharged()) {
		count_field->Enable(false);
	}
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
	action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
	unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());
}

void OldPropertiesWindow::createClassificationFields(wxFlexGridSizer* subsizer) {
	// item classification (12.81+)
	if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Classification"));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(edit_item->getClassification())));

		// item iter
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tier"));
		tier_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
		subsizer->Add(tier_field, wxSizerFlags(1).Expand());
	}
}

void OldPropertiesWindow::createDoorFields(wxFlexGridSizer* subsizer) {
	if (Door* door = dynamic_cast<Door*>(edit_item)) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Door ID"));
		door_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(door->getDoorID()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, 0xFF, door->getDoorID());
		if (!edit_tile || !edit_tile->isHouseTile() || !door->isRealDoor()) {
			door_id_field->Disable();
		}
		subsizer->Add(door_id_field, wxSizerFlags(1).Expand());
	}
}

void OldPropertiesWindow::createTeleportFields(wxFlexGridSizer* subsizer) {
	if (Teleport* teleport = dynamic_cast<Teleport*>(edit_item)) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Destination"));

		wxSizer* possizer = newd wxBoxSizer(wxHORIZONTAL);
		x_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getX()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, edit_map->getWidth(), teleport->getX());
		x_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(x_field, wxSizerFlags(3).Expand());
		y_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getY()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, edit_map->getHeight(), teleport->getY());
		y_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(y_field, wxSizerFlags(3).Expand());
		z_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getZ()), wxDefaultPosition, FromDIP(wxSize(-1, 20)), wxSP_ARROW_KEYS, 0, MAP_MAX_LAYER, teleport->getZ());
		z_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
		possizer->Add(z_field, wxSizerFlags(2).Expand());

		subsizer->Add(possizer, wxSizerFlags(1).Expand());
	}
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
		int x, y, z;
		const Editor* const editor = g_gui.GetCurrentEditor();
		if (TeleportService::handlePositionPaste(x, y, z, editor->map.getWidth(), editor->map.getHeight())) {
			x_field->SetValue(x);
			y_field->SetValue(y);
			z_field->SetValue(z);
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

		if (!PropertyValidator::validateItemProperties(this, new_uid, new_aid, new_tier)) {
			return;
		}

		if (door && !PropertyValidator::validateDoorProperties(this, edit_map, edit_tile, door, new_door_id)) {
			return;
		}

		if (teleport && !PropertyValidator::validateTeleportProperties(this, edit_map, new_dest)) {
			return;
		}

		// Done validating, set the values.
		PropertyApplier::applyItemProperties(edit_item, new_count, new_uid, new_aid, new_tier);
		if (door) {
			PropertyApplier::applyDoorProperties(door, new_door_id);
		}
		if (teleport) {
			PropertyApplier::applyTeleportProperties(teleport, new_dest);
		}
	}
	EndModal(1);
}

void OldPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
