//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/dialog_util.h"
#include <wx/app.h>
#include "ui/gui.h" 

#include "app/application.h"

long DialogUtil::PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString configsavename, uint32_t configsavevalue) {
	if (text.empty()) {
		return wxID_ANY;
	}

	wxMessageDialog dlg(parent, text, title, style);
	return dlg.ShowModal();
}

long DialogUtil::PopupDialog(wxString title, wxString text, long style, wxString configsavename, uint32_t configsavevalue) {
	// Need access to main frame for parent.
	return PopupDialog(static_cast<wxApp*>(wxTheApp)->GetTopWindow(), title, text, style, configsavename, configsavevalue);
}

void DialogUtil::ListDialog(wxWindow* parent, wxString title, const wxArrayString& param_items) {
	if (param_items.empty()) {
		return;
	}

	wxArrayString list_items(param_items);

	// Create the window
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxListBox* item_list = newd wxListBox(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
	item_list->SetMinSize(wxSize(500, 300));

	for (size_t i = 0; i != list_items.GetCount();) {
		wxString str = list_items[i];
		size_t pos = str.find("\n");
		if (pos != wxString::npos) {
			// Split string!
			item_list->Append(str.substr(0, pos));
			list_items[i] = str.substr(pos + 1);
			continue;
		}
		item_list->Append(list_items[i]);
		++i;
	}
	sizer->Add(item_list, 1, wxEXPAND);

	wxSizer* stdsizer = newd wxBoxSizer(wxHORIZONTAL);
	stdsizer->Add(newd wxButton(dlg, wxID_OK, "OK"), wxSizerFlags(1).Center());
	sizer->Add(stdsizer, wxSizerFlags(0).Center());

	dlg->SetSizerAndFit(sizer);

	// Show the window
	dlg->ShowModal();
	delete dlg;
}

void DialogUtil::ShowTextBox(wxWindow* parent, wxString title, wxString content) {
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text_field = newd wxTextCtrl(dlg, wxID_ANY, content, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	text_field->SetMinSize(wxSize(400, 550));
	topsizer->Add(text_field, wxSizerFlags(5).Expand());

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	choicesizer->Add(newd wxButton(dlg, wxID_CANCEL, "OK"), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(0).Center());
	dlg->SetSizerAndFit(topsizer);

	dlg->ShowModal();
}
