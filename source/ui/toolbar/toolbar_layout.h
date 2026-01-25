//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_TOOLBAR_LAYOUT_H_
#define RME_UI_TOOLBAR_TOOLBAR_LAYOUT_H_

#include <wx/aui/aui.h>
#include "ui/toolbar/toolbar_registry.h"

// Responsibility: Configure the AUI manager with the toolbars.
class ToolbarLayout {
public:
	static void Configure(wxAuiManager* manager, ToolbarRegistry* registry);
};

#endif // RME_UI_TOOLBAR_TOOLBAR_LAYOUT_H_
