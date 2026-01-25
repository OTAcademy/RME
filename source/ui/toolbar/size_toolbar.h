//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_SIZE_TOOLBAR_H_
#define RME_UI_TOOLBAR_SIZE_TOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/auibar.h>
#include "brushes/brush_enums.h"

class SizeToolBar : public wxEvtHandler {
public:
	SizeToolBar(wxWindow* parent);
	~SizeToolBar();

	wxAuiToolBar* GetToolbar() const {
		return toolbar;
	}

	void Update();
	void UpdateBrushSize(BrushShape shape, int size);
	void OnToolbarClick(wxCommandEvent& event);

	static const wxString PANE_NAME;

private:
	wxAuiToolBar* toolbar;
};

#endif
