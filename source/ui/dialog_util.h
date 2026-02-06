//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_DIALOG_UTIL_H_
#define RME_UI_DIALOG_UTIL_H_

#include <wx/wx.h>

class DialogUtil {
public:
	static long PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString configsavename = wxEmptyString, uint32_t configsavevalue = 0);
	static long PopupDialog(wxString title, wxString text, long style, wxString configsavename = wxEmptyString, uint32_t configsavevalue = 0);

	static void ListDialog(wxWindow* parent, wxString title, const std::vector<std::string>& vec);
	static void ListDialog(const wxString& title, const std::vector<std::string>& vec) {
		ListDialog(nullptr, title, vec);
	}

	static void ShowTextBox(wxWindow* parent, wxString title, wxString contents);
	static void ShowTextBox(const wxString& title, const wxString& contents) {
		ShowTextBox(nullptr, title, contents);
	}
};

#endif
