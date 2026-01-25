//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_PODIUM_PROPERTIES_WINDOW_H_
#define RME_PODIUM_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class PodiumPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	PodiumPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~PodiumPropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxSpinCtrl* tier_field;
	wxChoice* direction_field;

	wxCheckBox* show_outfit;
	wxCheckBox* show_mount;
	wxCheckBox* show_platform;
	wxSpinCtrl* look_type;
	wxSpinCtrl* look_head;
	wxSpinCtrl* look_body;
	wxSpinCtrl* look_legs;
	wxSpinCtrl* look_feet;
	wxSpinCtrl* look_addon;
	wxSpinCtrl* look_mount;
	wxSpinCtrl* look_mounthead;
	wxSpinCtrl* look_mountbody;
	wxSpinCtrl* look_mountlegs;
	wxSpinCtrl* look_mountfeet;

	DECLARE_EVENT_TABLE()
};

#endif
