//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_TOOLBAR_FACTORY_H_
#define RME_UI_TOOLBAR_TOOLBAR_FACTORY_H_

#include <memory>
#include <wx/window.h>
#include "ui/toolbar/toolbar_registry.h"

// Responsibility: Creates instances of all toolbars.
class ToolbarFactory {
public:
	static std::unique_ptr<ToolbarRegistry> CreateToolbars(wxWindow* parent);
};

#endif // RME_UI_TOOLBAR_TOOLBAR_FACTORY_H_
