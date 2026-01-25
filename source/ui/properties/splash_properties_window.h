//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_SPLASH_PROPERTIES_WINDOW_H
#define RME_SPLASH_PROPERTIES_WINDOW_H

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class SplashPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	SplashPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~SplashPropertiesWindow();

	void OnClickOK(wxCommandEvent& event);
	void OnClickCancel(wxCommandEvent& event);

protected:
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxChoice* splash_type_field;

	DECLARE_EVENT_TABLE()
};

#endif
