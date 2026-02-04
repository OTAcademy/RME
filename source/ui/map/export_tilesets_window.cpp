#include "app/main.h"
#include "ui/map/export_tilesets_window.h"

#include "editor/editor.h"
#include "editor/persistence/tileset_exporter.h"
#include "ui/gui_ids.h"
#include "app/application.h"

#include <wx/dirdlg.h>
#include <wx/textctrl.h>

// ============================================================================
// Export Tilesets window

BEGIN_EVENT_TABLE(ExportTilesetsWindow, wxDialog)
EVT_BUTTON(TILESET_FILE_BUTTON, ExportTilesetsWindow::OnClickBrowse)
EVT_BUTTON(wxID_OK, ExportTilesetsWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, ExportTilesetsWindow::OnClickCancel)
END_EVENT_TABLE()

ExportTilesetsWindow::ExportTilesetsWindow(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, "Export Tilesets", wxDefaultPosition, wxSize(400, 230)),
	editor(editor) {
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	// Error field
	error_field = newd wxStaticText(this, wxID_VIEW_DETAILS, "", wxDefaultPosition, wxDefaultSize);
	error_field->SetForegroundColour(*wxRED);
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(error_field, 0, wxALL, 5);
	sizer->Add(tmpsizer, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	// Output folder
	directory_text_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	directory_text_field->Bind(wxEVT_KEY_UP, &ExportTilesetsWindow::OnDirectoryChanged, this);
	directory_text_field->SetValue(wxString(g_settings.getString(Config::TILESET_EXPORT_DIR)));
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, "Output Folder");
	tmpsizer->Add(directory_text_field, 1, wxALL, 5);
	auto browseBtn = newd wxButton(this, TILESET_FILE_BUTTON, "Browse");
	browseBtn->SetToolTip("Browse for output directory");
	tmpsizer->Add(browseBtn, 0, wxALL, 5);
	sizer->Add(tmpsizer, 0, wxALL | wxEXPAND, 5);

	// File name
	file_name_text_field = newd wxTextCtrl(this, wxID_ANY, "tilesets", wxDefaultPosition, wxDefaultSize);
	file_name_text_field->Bind(wxEVT_KEY_UP, &ExportTilesetsWindow::OnFileNameChanged, this);
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, "File Name");
	tmpsizer->Add(file_name_text_field, 1, wxALL, 5);
	sizer->Add(tmpsizer, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	ok_button = newd wxButton(this, wxID_OK, "OK");
	ok_button->SetToolTip("Start export");
	tmpsizer->Add(ok_button, wxSizerFlags(1).Center());
	auto cancelBtn = newd wxButton(this, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Cancel");
	tmpsizer->Add(cancelBtn, wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxCENTER, 10);

	SetSizer(sizer);
	Layout();
	Centre(wxBOTH);
	CheckValues();
}

ExportTilesetsWindow::~ExportTilesetsWindow() = default;

void ExportTilesetsWindow::OnClickBrowse(wxCommandEvent& WXUNUSED(event)) {
	wxDirDialog dialog(nullptr, "Select the output folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dialog.ShowModal() == wxID_OK) {
		const wxString& directory = dialog.GetPath();
		directory_text_field->ChangeValue(directory);
	}
	CheckValues();
}

void ExportTilesetsWindow::OnDirectoryChanged(wxKeyEvent& event) {
	CheckValues();
	event.Skip();
}

void ExportTilesetsWindow::OnFileNameChanged(wxKeyEvent& event) {
	CheckValues();
	event.Skip();
}

void ExportTilesetsWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	g_settings.setString(Config::TILESET_EXPORT_DIR, directory_text_field->GetValue().ToStdString());

	FileName directory(directory_text_field->GetValue());
	std::string filename = file_name_text_field->GetValue().ToStdString();

	TilesetExporter::exportTilesets(directory, filename);

	EndModal(1);
}

void ExportTilesetsWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}

void ExportTilesetsWindow::CheckValues() {
	if (directory_text_field->IsEmpty()) {
		error_field->SetLabel("Type or select an output folder.");
		ok_button->Enable(false);
		return;
	}

	if (file_name_text_field->IsEmpty()) {
		error_field->SetLabel("Type a name for the file.");
		ok_button->Enable(false);
		return;
	}

	FileName directory(directory_text_field->GetValue());

	if (!directory.Exists()) {
		error_field->SetLabel("Output folder not found.");
		ok_button->Enable(false);
		return;
	}

	if (!directory.IsDirWritable()) {
		error_field->SetLabel("Output folder is not writable.");
		ok_button->Enable(false);
		return;
	}

	error_field->SetLabel(wxEmptyString);
	ok_button->Enable(true);
}
