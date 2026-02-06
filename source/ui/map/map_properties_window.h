#ifndef RME_UI_MAP_MAP_PROPERTIES_WINDOW_H_
#define RME_UI_MAP_MAP_PROPERTIES_WINDOW_H_

#include "app/main.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>

class MapTab;
class Editor;

class MapPropertiesWindow : public wxDialog {
public:
	MapPropertiesWindow(wxWindow* parent, MapTab* tab, Editor& editor);
	virtual ~MapPropertiesWindow();

	void OnChangeVersion(wxCommandEvent&);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	void UpdateProtocolList();

	MapTab* view;
	Editor& editor;
	wxSpinCtrl* height_spin;
	wxSpinCtrl* width_spin;
	wxChoice* version_choice;
	wxChoice* protocol_choice;
	wxTextCtrl* description_ctrl;
	wxTextCtrl* house_filename_ctrl;
	wxTextCtrl* spawn_filename_ctrl;
};

#endif
