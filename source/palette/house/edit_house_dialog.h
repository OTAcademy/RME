//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EDIT_HOUSE_DIALOG_H_
#define RME_EDIT_HOUSE_DIALOG_H_

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>

#include <vector>
#include <cstdint>

class Map;
class House;

class EditHouseDialog : public wxDialog {
public:
	EditHouseDialog(wxWindow* parent, Map* map, House* house);
	virtual ~EditHouseDialog();

	void OnFocusChange(wxFocusEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	Map* map;
	House* what_house;

	wxString house_name, house_id, house_rent;

	wxTextCtrl* name_field;
	wxChoice* town_id_field;
	wxSpinCtrl* id_field;
	wxTextCtrl* rent_field;
	wxCheckBox* guildhall_field;

	std::vector<uint32_t> town_ids_;
};

#endif
