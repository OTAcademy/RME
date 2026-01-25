//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/podium_properties_window.h"

#include "game/complexitem.h"
#include "game/creature.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"

static constexpr int OUTFIT_COLOR_MAX = 133;

BEGIN_EVENT_TABLE(PodiumPropertiesWindow, wxDialog)
EVT_BUTTON(wxID_OK, PodiumPropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, PodiumPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

PodiumPropertiesWindow::PodiumPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Podium Properties", map, tile_parent, item, pos) {
	ASSERT(edit_item);
	Podium* podium = dynamic_cast<Podium*>(edit_item);
	ASSERT(podium);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Podium Properties");

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
	action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
	unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

	if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Classification"));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(item->getClassification())));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tier"));
		tier_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
		subsizer->Add(tier_field, wxSizerFlags(1).Expand());
	} else {
		tier_field = nullptr;
	}

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
	direction_field = newd wxChoice(this, wxID_ANY);

	for (Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
		direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
	}
	direction_field->SetSelection(static_cast<Direction>(podium->getDirection()));
	subsizer->Add(direction_field, wxSizerFlags(1).Expand());

	show_outfit = newd wxCheckBox(this, wxID_ANY, "Show outfit");
	show_outfit->SetValue(podium->hasShowOutfit());
	show_outfit->SetToolTip("Display outfit on the podium.");
	subsizer->Add(show_outfit, 0, wxLEFT | wxTOP, 5);
	subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

	show_mount = newd wxCheckBox(this, wxID_ANY, "Show mount");
	show_mount->SetValue(podium->hasShowMount());
	show_mount->SetToolTip("Display mount on the podium.");
	subsizer->Add(show_mount, 0, wxLEFT | wxTOP, 5);
	subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

	show_platform = newd wxCheckBox(this, wxID_ANY, "Show platform");
	show_platform->SetValue(podium->hasShowPlatform());
	show_platform->SetToolTip("Display the podium platform.");
	subsizer->Add(show_platform, 0, wxLEFT | wxTOP, 5);
	subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

	wxFlexGridSizer* outfitContainer = newd wxFlexGridSizer(2, 10, 10);
	const Outfit& outfit = podium->getOutfit();

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Outfit"));
	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "LookType"));
	look_type = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookType), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookType);
	outfitContainer->Add(look_type, wxSizerFlags(3).Expand());

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
	look_head = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookHead);
	outfitContainer->Add(look_head, wxSizerFlags(3).Expand());

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
	look_body = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookBody);
	outfitContainer->Add(look_body, wxSizerFlags(3).Expand());

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
	look_legs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookLegs);
	outfitContainer->Add(look_legs, wxSizerFlags(3).Expand());

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
	look_feet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookFeet);
	outfitContainer->Add(look_feet, wxSizerFlags(3).Expand());

	outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Addons"));
	look_addon = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookAddon), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 3, outfit.lookAddon);
	outfitContainer->Add(look_addon, wxSizerFlags(3).Expand());

	wxFlexGridSizer* mountContainer = newd wxFlexGridSizer(2, 10, 10);
	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Mount"));
	mountContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "LookMount"));
	look_mount = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMount), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookMount);
	mountContainer->Add(look_mount, wxSizerFlags(3).Expand());

	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
	look_mounthead = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountHead);
	mountContainer->Add(look_mounthead, wxSizerFlags(3).Expand());

	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
	look_mountbody = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountBody);
	mountContainer->Add(look_mountbody, wxSizerFlags(3).Expand());

	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
	look_mountlegs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountLegs);
	mountContainer->Add(look_mountlegs, wxSizerFlags(3).Expand());

	mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
	look_mountfeet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountFeet);
	mountContainer->Add(look_mountfeet, wxSizerFlags(3).Expand());

	wxFlexGridSizer* propertiesContainer = newd wxFlexGridSizer(3, 10, 10);
	propertiesContainer->Add(subsizer, wxSizerFlags(1).Expand());
	propertiesContainer->Add(outfitContainer, wxSizerFlags(1).Expand());
	propertiesContainer->Add(mountContainer, wxSizerFlags(1).Expand());
	boxsizer->Add(propertiesContainer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

PodiumPropertiesWindow::~PodiumPropertiesWindow() {
	if (direction_field) {
		for (uint32_t i = 0; i < direction_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(direction_field->GetClientData(i));
		}
	}
}

void PodiumPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	Podium* podium = dynamic_cast<Podium*>(edit_item);

	int new_uid = unique_id_field->GetValue();
	int new_aid = action_id_field->GetValue();
	int new_tier = tier_field ? tier_field->GetValue() : 0;

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

	int newLookType = look_type->GetValue();
	int newMount = look_mount->GetValue();

	if (newLookType < 0 || newLookType > 0xFFFF || newMount < 0 || newMount > 0xFFFF) {
		DialogUtil::PopupDialog(this, "Error", "LookType and Mount must be between 0 and 65535.", wxOK);
		return;
	}

	int newHead = look_head->GetValue();
	int newBody = look_body->GetValue();
	int newLegs = look_legs->GetValue();
	int newFeet = look_feet->GetValue();
	int newMountHead = look_mounthead->GetValue();
	int newMountBody = look_mountbody->GetValue();
	int newMountLegs = look_mountlegs->GetValue();
	int newMountFeet = look_mountfeet->GetValue();

	if (newHead < 0 || newHead > OUTFIT_COLOR_MAX || newBody < 0 || newBody > OUTFIT_COLOR_MAX || newLegs < 0 || newLegs > OUTFIT_COLOR_MAX || newFeet < 0 || newFeet > OUTFIT_COLOR_MAX || newMountHead < 0 || newMountHead > OUTFIT_COLOR_MAX || newMountBody < 0 || newMountBody > OUTFIT_COLOR_MAX || newMountLegs < 0 || newMountLegs > OUTFIT_COLOR_MAX || newMountFeet < 0 || newMountFeet > OUTFIT_COLOR_MAX) {
		wxString response = "Outfit and mount colors must be between 0 and ";
		response << i2ws(OUTFIT_COLOR_MAX) << ".";
		DialogUtil::PopupDialog(this, "Error", response, wxOK);
		return;
	}

	int newAddon = look_addon->GetValue();
	if (newAddon < 0 || newAddon > 3) {
		DialogUtil::PopupDialog(this, "Error", "Addons value must be between 0 and 3.", wxOK);
		return;
	}

	Outfit newOutfit;
	newOutfit.lookType = newLookType;
	newOutfit.lookHead = newHead;
	newOutfit.lookBody = newBody;
	newOutfit.lookLegs = newLegs;
	newOutfit.lookFeet = newFeet;
	newOutfit.lookAddon = newAddon;
	newOutfit.lookMount = newMount;
	newOutfit.lookMountHead = newMountHead;
	newOutfit.lookMountBody = newMountBody;
	newOutfit.lookMountLegs = newMountLegs;
	newOutfit.lookMountFeet = newMountFeet;

	podium->setShowOutfit(show_outfit->GetValue());
	podium->setShowMount(show_mount->GetValue());
	podium->setShowPlatform(show_platform->GetValue());

	int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(direction_field->GetSelection()));
	if (new_dir) {
		podium->setDirection((Direction)*new_dir);
	}

	podium->setOutfit(newOutfit);
	edit_item->setUniqueID(new_uid);
	edit_item->setActionID(new_aid);
	edit_item->setTier(new_tier);

	EndModal(1);
}

void PodiumPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
