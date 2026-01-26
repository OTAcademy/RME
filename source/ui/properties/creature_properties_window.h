//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CREATURE_PROPERTIES_WINDOW_H_
#define RME_CREATURE_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class CreaturePropertiesWindow : public ObjectPropertiesWindowBase {
public:
	CreaturePropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Creature* creature, wxPoint pos = wxDefaultPosition);
	virtual ~CreaturePropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	wxSpinCtrl* count_field;
	wxChoice* direction_field;
};

#endif
