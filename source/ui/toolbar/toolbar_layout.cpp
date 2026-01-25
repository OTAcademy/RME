//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/toolbar_layout.h"

void ToolbarLayout::Configure(wxAuiManager* manager, ToolbarRegistry* registry) {
	if (!manager || !registry) {
		return;
	}

	// Add panes
	if (auto tb = registry->GetStandardToolbar()) {
		manager->AddPane(tb->GetToolbar(), wxAuiPaneInfo().Name(StandardToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	}
	if (auto tb = registry->GetBrushToolbar()) {
		manager->AddPane(tb->GetToolbar(), wxAuiPaneInfo().Name(BrushToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));
	}
	if (auto tb = registry->GetSizeToolbar()) { // Note: Size toolbar was position 3 in original code
		manager->AddPane(tb->GetToolbar(), wxAuiPaneInfo().Name(SizeToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(3).Floatable(false));
	}
	if (auto tb = registry->GetPositionToolbar()) { // Note: Position toolbar was position 4 in original code
		manager->AddPane(tb->GetToolbar(), wxAuiPaneInfo().Name(PositionToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(4).Floatable(false));
	}
	if (auto tb = registry->GetLightToolbar()) {
		manager->AddPane(tb->GetToolbar(), wxAuiPaneInfo().Name(LightToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(5).Floatable(false));
	}
}
