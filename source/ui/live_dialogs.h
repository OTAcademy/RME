//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_LIVE_DIALOGS_H_
#define RME_UI_LIVE_DIALOGS_H_

#include <wx/window.h>

class Editor;

class LiveDialogs {
public:
	static void ShowHostDialog(wxWindow* parent, Editor* editor);
	static void ShowJoinDialog(wxWindow* parent);
};

#endif
