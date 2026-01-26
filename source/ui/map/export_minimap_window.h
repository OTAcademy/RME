#ifndef RME_UI_MAP_EXPORT_MINIMAP_WINDOW_H_
#define RME_UI_MAP_EXPORT_MINIMAP_WINDOW_H_

#include "app/main.h"

#include <wx/dialog.h>

class Editor;
class wxStaticText;
class wxTextCtrl;
class wxChoice;
class wxSpinCtrl;
class wxButton;

/**
 * The export minimap dialog, select output path and what floors to export.
 */
class ExportMiniMapWindow : public wxDialog {
public:
	ExportMiniMapWindow(wxWindow* parent, Editor& editor);
	virtual ~ExportMiniMapWindow();

	void OnClickBrowse(wxCommandEvent&);
	void OnDirectoryChanged(wxKeyEvent&);
	void OnFileNameChanged(wxKeyEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnExportTypeChange(wxCommandEvent&);

protected:
	void CheckValues();

	Editor& editor;

	wxStaticText* error_field;
	wxTextCtrl* directory_text_field;
	wxTextCtrl* file_name_text_field;
	wxChoice* floor_options;
	wxSpinCtrl* floor_number;
	wxButton* ok_button;

	DECLARE_EVENT_TABLE();
};

#endif
