//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/container_properties_window.h"

#include "game/complexitem.h"
#include "ui/dialog_util.h"
#include "ui/properties/property_validator.h"
#include "app/application.h"

/*
BEGIN_EVENT_TABLE(ContainerPropertiesWindow, wxDialog)
EVT_BUTTON(wxID_OK, ContainerPropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, ContainerPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()
*/

ContainerPropertiesWindow::ContainerPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Container Properties", map, tile_parent, item, pos),
	action_id_field(nullptr),
	unique_id_field(nullptr) {
	ASSERT(edit_item);
	Container* container = dynamic_cast<Container*>(edit_item);
	ASSERT(container);

	Bind(wxEVT_BUTTON, &ContainerPropertiesWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &ContainerPropertiesWindow::OnClickCancel, this, wxID_CANCEL);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Container Properties");

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

	boxsizer->Add(subsizer, wxSizerFlags(0).Expand());

	// Now we add the subitems!
	wxSizer* contents_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Contents");

	bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
	wxSizer* horizontal_sizer = nullptr;
	int32_t maxColumns;
	if (use_large_sprites) {
		maxColumns = 6;
	} else {
		maxColumns = 12;
	}

	for (uint32_t index = 0; index < container->getVolume(); ++index) {
		if (!horizontal_sizer) {
			horizontal_sizer = newd wxBoxSizer(wxHORIZONTAL);
		}

		Item* sub_item = container->getItem(index);
		ContainerItemButton* containerItemButton = newd ContainerItemButton(this, use_large_sprites, index, map, sub_item);

		container_items.push_back(containerItemButton);
		horizontal_sizer->Add(containerItemButton);

		if (((index + 1) % maxColumns) == 0) {
			contents_sizer->Add(horizontal_sizer);
			horizontal_sizer = nullptr;
		}
	}

	if (horizontal_sizer != nullptr) {
		contents_sizer->Add(horizontal_sizer);
	}

	boxsizer->Add(contents_sizer, wxSizerFlags(2).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

ContainerPropertiesWindow::~ContainerPropertiesWindow() {
	//
}

void ContainerPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	int new_uid = unique_id_field->GetValue();
	int new_aid = action_id_field->GetValue();

	if (!PropertyValidator::validateItemProperties(this, new_uid, new_aid, 0)) {
		return;
	}

	edit_item->setUniqueID(new_uid);
	edit_item->setActionID(new_aid);
	EndModal(1);
}

void ContainerPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}

void ContainerPropertiesWindow::Update() {
	Container* container = dynamic_cast<Container*>(edit_item);
	if (container) {
		for (uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}
