//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/container_properties_window.h"

#include "game/complexitem.h"
#include "ui/dialog_util.h"
#include "ui/properties/property_validator.h"
#include "app/application.h"
#include "ui/find_item_window.h"
#include "ui/gui_ids.h"
#include "ui/properties/properties_window.h"
#include "ui/properties/old_properties_window.h"

ContainerPropertiesWindow::ContainerPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Container Properties", map, tile_parent, item, pos),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	last_clicked_button(nullptr) {
	ASSERT(edit_item);
	Container* container = dynamic_cast<Container*>(edit_item);
	ASSERT(container);

	Bind(wxEVT_BUTTON, &ContainerPropertiesWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &ContainerPropertiesWindow::OnClickCancel, this, wxID_CANCEL);

	Bind(wxEVT_MENU, &ContainerPropertiesWindow::OnAddItem, this, CONTAINER_POPUP_MENU_ADD);
	Bind(wxEVT_MENU, &ContainerPropertiesWindow::OnEditItem, this, CONTAINER_POPUP_MENU_EDIT);
	Bind(wxEVT_MENU, &ContainerPropertiesWindow::OnRemoveItem, this, CONTAINER_POPUP_MENU_REMOVE);

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
		containerItemButton->Bind(wxEVT_BUTTON, &ContainerPropertiesWindow::OnContainerItemClick, this);
		containerItemButton->Bind(wxEVT_RIGHT_UP, &ContainerPropertiesWindow::OnContainerItemRightClick, this);

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
	Layout();
	wxDialog::Update();
}

void ContainerPropertiesWindow::OnContainerItemClick(wxCommandEvent& event) {
	ContainerItemButton* button = dynamic_cast<ContainerItemButton*>(event.GetEventObject());
	if (!button) {
		return;
	}

	last_clicked_button = button;

	if (button->getItem()) {
		OnEditItem(event);
	} else {
		OnAddItem(event);
	}
}

void ContainerPropertiesWindow::OnContainerItemRightClick(wxMouseEvent& event) {
	ContainerItemButton* button = dynamic_cast<ContainerItemButton*>(event.GetEventObject());
	if (!button) {
		return;
	}

	last_clicked_button = button;

	wxMenu menu;
	if (button->getItem()) {
		menu.Append(CONTAINER_POPUP_MENU_EDIT, "&Edit Item");
		menu.Append(CONTAINER_POPUP_MENU_ADD, "&Add Item");
		menu.Append(CONTAINER_POPUP_MENU_REMOVE, "&Remove Item");
	} else {
		menu.Append(CONTAINER_POPUP_MENU_ADD, "&Add Item");
	}

	Container* container = dynamic_cast<Container*>(edit_item);
	if (container && container->getVolume() <= (int)container->getVector().size()) {
		if (wxMenuItem* addItem = menu.FindItem(CONTAINER_POPUP_MENU_ADD)) {
			addItem->Enable(false);
		}
	}

	PopupMenu(&menu);
}

void ContainerPropertiesWindow::OnAddItem(wxCommandEvent& WXUNUSED(event)) {
	if (!last_clicked_button) {
		return;
	}

	Container* container = dynamic_cast<Container*>(edit_item);
	if (!container) {
		return;
	}

	FindItemDialog dialog(this, "Select Item");
	if (dialog.ShowModal() == wxID_OK) {
		uint16_t item_id = dialog.getResultID();
		if (item_id != 0) {
			ItemVector& contents = container->getVector();
			uint32_t index = last_clicked_button->getIndex();

			std::unique_ptr<Item> new_item(Item::Create(item_id)); // Wrap locally
			if (new_item) {
				if (index < contents.size()) {
					contents.insert(contents.begin() + index, new_item.release()); // Release ownership to vector
				} else {
					contents.push_back(new_item.release()); // Release ownership to vector
				}
			}
			Update();
		}
	}
}

void ContainerPropertiesWindow::OnEditItem(wxCommandEvent& WXUNUSED(event)) {
	if (!last_clicked_button || !last_clicked_button->getItem()) {
		return;
	}

	Item* sub_item = last_clicked_button->getItem();
	wxPoint newDialogAt = GetPosition() + wxPoint(20, 20);

	wxDialog* d;
	if (edit_map->getVersion().otbm >= MAP_OTBM_4) {
		d = newd PropertiesWindow(this, edit_map, nullptr, sub_item, newDialogAt);
	} else {
		d = newd OldPropertiesWindow(this, edit_map, nullptr, sub_item, newDialogAt);
	}

	d->ShowModal();
	d->Destroy();
	Update();
}

void ContainerPropertiesWindow::OnRemoveItem(wxCommandEvent& WXUNUSED(event)) {
	if (!last_clicked_button || !last_clicked_button->getItem()) {
		return;
	}

	int32_t ret = DialogUtil::PopupDialog(this, "Remove Item", "Are you sure you want to remove this item from the container?", wxYES | wxNO);

	if (ret != wxID_YES) {
		return;
	}

	Container* container = dynamic_cast<Container*>(edit_item);
	if (!container) {
		return;
	}

	ItemVector& contents = container->getVector();
	Item* to_remove = last_clicked_button->getItem();

	auto it = std::find(contents.begin(), contents.end(), to_remove);
	if (it != contents.end()) {
		std::unique_ptr<Item> item_ptr(*it); // Transfer ownership to unique_ptr for safe deletion
		contents.erase(it);
	}

	Update();
}
