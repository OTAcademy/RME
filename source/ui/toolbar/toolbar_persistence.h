//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_TOOLBAR_PERSISTENCE_H_
#define RME_UI_TOOLBAR_TOOLBAR_PERSISTENCE_H_

#include <wx/aui/aui.h>
#include "ui/main_toolbar.h"

class ToolbarPersistence {
public:
	static void LoadPerspective(wxAuiManager* manager, MainToolBar* mainToolbar);
	static void SavePerspective(wxAuiManager* manager, MainToolBar* mainToolbar);
};

#endif // RME_UI_TOOLBAR_TOOLBAR_PERSISTENCE_H_
