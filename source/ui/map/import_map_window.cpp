#include "app/main.h"
#include "ui/map/import_map_window.h"

#include "editor/editor.h"
#include "editor/persistence/editor_persistence.h"
#include "ui/dialog_util.h"
#include "ui/gui_ids.h"
#include "map/map.h"

#include <wx/filedlg.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>

// ============================================================================
// Map Import Window

BEGIN_EVENT_TABLE(ImportMapWindow, wxDialog)
EVT_BUTTON(MAP_WINDOW_FILE_BUTTON, ImportMapWindow::OnClickBrowse)
EVT_BUTTON(wxID_OK, ImportMapWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, ImportMapWindow::OnClickCancel)
END_EVENT_TABLE()

ImportMapWindow::ImportMapWindow(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, "Import Map", wxDefaultPosition, wxSize(350, 315)),
	editor(editor) {
	wxBoxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer* tmpsizer;

	// File
	tmpsizer = newd wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Map File"), wxHORIZONTAL);
	file_text_field = newd wxTextCtrl(tmpsizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxSize(230, 23));
	tmpsizer->Add(file_text_field, 0, wxALL, 5);
	wxButton* browse_button = newd wxButton(tmpsizer->GetStaticBox(), MAP_WINDOW_FILE_BUTTON, "Browse...", wxDefaultPosition, wxSize(80, 23));
	tmpsizer->Add(browse_button, 0, wxALL, 5);
	sizer->Add(tmpsizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);

	// Import offset
	tmpsizer = newd wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Import Offset"), wxHORIZONTAL);
	tmpsizer->Add(newd wxStaticText(tmpsizer->GetStaticBox(), wxID_ANY, "Offset X:"), 0, wxALL | wxEXPAND, 5);
	x_offset_ctrl = newd wxSpinCtrl(tmpsizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, 23), wxSP_ARROW_KEYS, -MAP_MAX_HEIGHT, MAP_MAX_HEIGHT);
	tmpsizer->Add(x_offset_ctrl, 0, wxALL, 5);
	tmpsizer->Add(newd wxStaticText(tmpsizer->GetStaticBox(), wxID_ANY, "Offset Y:"), 0, wxALL, 5);
	y_offset_ctrl = newd wxSpinCtrl(tmpsizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, 23), wxSP_ARROW_KEYS, -MAP_MAX_HEIGHT, MAP_MAX_HEIGHT);
	tmpsizer->Add(y_offset_ctrl, 0, wxALL, 5);
	sizer->Add(tmpsizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// Import options
	wxArrayString house_choices;
	house_choices.Add("Smart Merge");
	house_choices.Add("Insert");
	house_choices.Add("Merge");
	house_choices.Add("Don't Import");

	// House options
	tmpsizer = newd wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "House Import Behaviour"), wxVERTICAL);
	house_options = newd wxChoice(tmpsizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, house_choices);
	house_options->SetSelection(0);
	tmpsizer->Add(house_options, 0, wxALL | wxEXPAND, 5);
	sizer->Add(tmpsizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// Import options
	wxArrayString spawn_choices;
	spawn_choices.Add("Merge");
	spawn_choices.Add("Don't Import");

	// Spawn options
	tmpsizer = newd wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Spawn Import Behaviour"), wxVERTICAL);
	spawn_options = newd wxChoice(tmpsizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, spawn_choices);
	spawn_options->SetSelection(0);
	tmpsizer->Add(spawn_options, 0, wxALL | wxEXPAND, 5);
	sizer->Add(tmpsizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// OK/Cancel buttons
	wxBoxSizer* buttons = newd wxBoxSizer(wxHORIZONTAL);
	buttons->Add(newd wxButton(this, wxID_OK, "Ok"), 0, wxALL, 5);
	buttons->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
	sizer->Add(buttons, wxSizerFlags(1).Center());

	SetSizer(sizer);
	Layout();
	Centre(wxBOTH);
}

ImportMapWindow::~ImportMapWindow() = default;

void ImportMapWindow::OnClickBrowse(wxCommandEvent& WXUNUSED(event)) {
	wxFileDialog dialog(this, "Import...", "", "", "*.otbm", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int ok = dialog.ShowModal();

	if (ok == wxID_OK) {
		file_text_field->ChangeValue(dialog.GetPath());
	}
}

void ImportMapWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (Validate() && TransferDataFromWindow()) {
		wxFileName fn = file_text_field->GetValue();
		if (!fn.FileExists()) {
			DialogUtil::PopupDialog(this, "Error", "The specified map file doesn't exist", wxOK);
			return;
		}

		ImportType spawn_import_type = IMPORT_DONT;
		ImportType house_import_type = IMPORT_DONT;

		switch (spawn_options->GetSelection()) {
			case 0:
				spawn_import_type = IMPORT_MERGE;
				break;
			case 1:
				spawn_import_type = IMPORT_DONT;
				break;
		}

		switch (house_options->GetSelection()) {
			case 0:
				house_import_type = IMPORT_SMART_MERGE;
				break;
			case 1:
				house_import_type = IMPORT_MERGE;
				break;
			case 2:
				house_import_type = IMPORT_INSERT;
				break;
			case 3:
				house_import_type = IMPORT_DONT;
				break;
		}

		EndModal(1);

		EditorPersistence::importMap(editor, fn, x_offset_ctrl->GetValue(), y_offset_ctrl->GetValue(), house_import_type, spawn_import_type);
	}
}

void ImportMapWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}
