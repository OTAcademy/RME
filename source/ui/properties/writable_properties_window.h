//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_WRITABLE_PROPERTIES_WINDOW_H
#define RME_WRITABLE_PROPERTIES_WINDOW_H

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

class WritablePropertiesWindow : public ObjectPropertiesWindowBase {
public:
	WritablePropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint pos = wxDefaultPosition);
	virtual ~WritablePropertiesWindow();

	void OnClickOK(wxCommandEvent& event);
	void OnClickCancel(wxCommandEvent& event);

protected:
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxTextCtrl* text_field;
};

#endif
