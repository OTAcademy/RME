//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CONTAINER_PROPERTIES_WINDOW_H_
#define RME_CONTAINER_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include "ui/dcbutton.h"
#include "game/item.h"
#include "ui/properties/object_properties_base.h"

class ContainerItemButton : public DCButton {
public:
	ContainerItemButton(wxWindow* parent, bool large, uint32_t index, const Map* map, Item* item) :
		DCButton(parent, wxID_ANY, wxDefaultPosition, DC_BTN_NORMAL, (large ? RENDER_SIZE_32x32 : RENDER_SIZE_16x16), (item ? item->getClientID() : 0)),
		index(index),
		item(item) { }

	void setItem(Item* new_item) {
		item = new_item;
		SetSprite(item ? item->getClientID() : 0);
	}

	Item* getItem() const {
		return item;
	}
	uint32_t getIndex() const {
		return index;
	}

protected:
	uint32_t index;
	Item* item;
};

class ContainerPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	ContainerPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~ContainerPropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void Update() override;
	void OnContainerItemClick(wxCommandEvent& event);
	void OnContainerItemRightClick(wxMouseEvent& event);

	void OnAddItem(wxCommandEvent& event);
	void OnEditItem(wxCommandEvent& event);
	void OnRemoveItem(wxCommandEvent& event);

protected:
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	std::vector<ContainerItemButton*> container_items;

	ContainerItemButton* last_clicked_button;
};

#endif
