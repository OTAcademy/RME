//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/toolbar_factory.h"

std::unique_ptr<ToolbarRegistry> ToolbarFactory::CreateToolbars(wxWindow* parent) {
	auto registry = std::make_unique<ToolbarRegistry>();

	// Use newd macro as per project convention if applicable, or just new.
	// The original code used `newd`.

	registry->SetStandardToolbar(std::make_unique<StandardToolBar>(parent));
	registry->SetBrushToolbar(std::make_unique<BrushToolBar>(parent));
	registry->SetPositionToolbar(std::make_unique<PositionToolBar>(parent));
	registry->SetSizeToolbar(std::make_unique<SizeToolBar>(parent));
	registry->SetLightToolbar(std::make_unique<LightToolBar>(parent));

	return registry;
}
