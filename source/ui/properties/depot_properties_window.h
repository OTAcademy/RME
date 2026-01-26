//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DEPOT_PROPERTIES_WINDOW_H
#define RME_DEPOT_PROPERTIES_WINDOW_H

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class DepotPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	DepotPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~DepotPropertiesWindow();

	void OnClickOK(wxCommandEvent& event);
	void OnClickCancel(wxCommandEvent& event);

protected:
	wxChoice* depot_id_field;
};

#endif
