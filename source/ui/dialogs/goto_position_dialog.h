//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_GotoPositionDialog_H
#define RME_GotoPositionDialog_H

#include "app/main.h"

class PositionCtrl;
class Editor;

/**
 * Go to position dialog
 * Allows entry of 3 coordinates and goes there instantly
 */
class GotoPositionDialog : public wxDialog {
public:
	GotoPositionDialog(wxWindow* parent, Editor& editor);
	~GotoPositionDialog() { }

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	Editor& editor;
	PositionCtrl* posctrl;
};

#endif
