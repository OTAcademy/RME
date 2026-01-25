//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_STANDARD_TOOLBAR_H_
#define RME_UI_TOOLBAR_STANDARD_TOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/auibar.h>

class StandardToolBar : public wxEvtHandler {
public:
	StandardToolBar(wxWindow* parent);
	~StandardToolBar();

	wxAuiToolBar* GetToolbar() const {
		return toolbar;
	}
	void Update();

	void OnButtonClick(wxCommandEvent& event);

	static const wxString PANE_NAME;

private:
	wxAuiToolBar* toolbar;
};

#endif // RME_UI_TOOLBAR_STANDARD_TOOLBAR_H_
