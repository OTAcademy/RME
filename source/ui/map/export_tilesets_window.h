#ifndef RME_UI_MAP_EXPORT_TILESETS_WINDOW_H_
#define RME_UI_MAP_EXPORT_TILESETS_WINDOW_H_

#include "app/main.h"

#include <wx/dialog.h>

class Editor;
class wxStaticText;
class wxTextCtrl;
class wxButton;

/**
 * The export tilesets dialog, select output path.
 */
class ExportTilesetsWindow : public wxDialog {
public:
	ExportTilesetsWindow(wxWindow* parent, Editor& editor);
	virtual ~ExportTilesetsWindow();

	void OnClickBrowse(wxCommandEvent&);
	void OnDirectoryChanged(wxKeyEvent&);
	void OnFileNameChanged(wxKeyEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	void CheckValues();

	Editor& editor;

	wxStaticText* error_field;
	wxTextCtrl* directory_text_field;
	wxTextCtrl* file_name_text_field;
	wxButton* ok_button;

	DECLARE_EVENT_TABLE();
};

#endif
