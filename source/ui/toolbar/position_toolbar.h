//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_POSITION_TOOLBAR_H_
#define RME_UI_TOOLBAR_POSITION_TOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/auibar.h>
#include "ui/numbertextctrl.h"

class PositionToolBar : public wxEvtHandler {
public:
	PositionToolBar(wxWindow* parent);
	~PositionToolBar();

	wxAuiToolBar* GetToolbar() const {
		return toolbar;
	}
	void Update();

	void OnGoClick(wxCommandEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnPaste(wxClipboardTextEvent& event);

	static const wxString PANE_NAME;

private:
	wxAuiToolBar* toolbar;
	NumberTextCtrl* x_control;
	NumberTextCtrl* y_control;
	NumberTextCtrl* z_control;
	wxButton* go_button;
};

#endif
