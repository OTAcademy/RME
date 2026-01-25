//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_BRUSH_TOOLBAR_H_
#define RME_UI_TOOLBAR_BRUSH_TOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/auibar.h>
#include "brushes/brush_enums.h"

class BrushToolBar : public wxEvtHandler {
public:
	BrushToolBar(wxWindow* parent);
	~BrushToolBar();

	wxAuiToolBar* GetToolbar() const {
		return toolbar;
	}

	// Updates both enablement and selection state
	void Update();

	void OnToolbarClick(wxCommandEvent& event);

	static const wxString PANE_NAME;

private:
	wxAuiToolBar* toolbar;
};

#endif
