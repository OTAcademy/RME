#ifndef RME_UI_MAP_IMPORT_MAP_WINDOW_H_
#define RME_UI_MAP_IMPORT_MAP_WINDOW_H_

#include "app/main.h"

#include <wx/dialog.h>

class Editor;
class wxTextCtrl;
class wxSpinCtrl;
class wxChoice;

/**
 * The import map dialog
 * Allows selection of file path, offset and some weird options.
 */
class ImportMapWindow : public wxDialog {
public:
	ImportMapWindow(wxWindow* parent, Editor& editor);
	virtual ~ImportMapWindow();

	void OnClickBrowse(wxCommandEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	Editor& editor;

	wxTextCtrl* file_text_field;
	wxSpinCtrl* x_offset_ctrl;
	wxSpinCtrl* y_offset_ctrl;

	wxChoice* house_options;
	wxChoice* spawn_options;
};

#endif
