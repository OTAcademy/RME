//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CONTAINER_PROPERTIES_WINDOW_H_
#define RME_CONTAINER_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class ContainerItemButton;

class ContainerPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	ContainerPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~ContainerPropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void Update() override;

protected:
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	std::vector<ContainerItemButton*> container_items;

	DECLARE_EVENT_TABLE()
};

#endif
