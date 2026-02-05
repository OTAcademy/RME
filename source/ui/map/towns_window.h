#ifndef RME_UI_MAP_TOWNS_WINDOW_H_
#define RME_UI_MAP_TOWNS_WINDOW_H_

#include "app/main.h"
#include <wx/wx.h>
#include <vector>
#include <memory>

class Editor;
class Town;
class PositionCtrl;

class EditTownsDialog : public wxDialog {
public:
	EditTownsDialog(wxWindow* parent, Editor& editor);
	virtual ~EditTownsDialog();

	void OnListBoxChange(wxCommandEvent&);
	void OnClickSelectTemplePosition(wxCommandEvent&);
	void OnClickAdd(wxCommandEvent&);
	void OnClickRemove(wxCommandEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	void BuildListBox(bool doselect);
	void UpdateSelection(int new_selection);

	Editor& editor;

	std::vector<std::unique_ptr<Town>> town_list;
	uint32_t max_town_id;

	wxListBox* town_listbox;
	wxString town_name, town_id;

	wxTextCtrl* name_field;
	wxTextCtrl* id_field;

	PositionCtrl* temple_position;
	wxButton* remove_button;
	wxButton* select_position_button;

	DECLARE_EVENT_TABLE();
};

#endif
