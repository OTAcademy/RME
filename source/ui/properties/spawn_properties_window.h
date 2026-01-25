//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_SPAWN_PROPERTIES_WINDOW_H_
#define RME_SPAWN_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class SpawnPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	SpawnPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Spawn* spawn, wxPoint pos = wxDefaultPosition);
	virtual ~SpawnPropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	wxSpinCtrl* count_field;

	DECLARE_EVENT_TABLE()
};

#endif
