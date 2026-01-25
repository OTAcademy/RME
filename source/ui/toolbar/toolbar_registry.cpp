//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/toolbar_registry.h"

ToolbarRegistry::ToolbarRegistry() :
	standard_toolbar(nullptr),
	brush_toolbar(nullptr),
	position_toolbar(nullptr),
	size_toolbar(nullptr),
	light_toolbar(nullptr) {
}

ToolbarRegistry::~ToolbarRegistry() {
	// Smart pointers handle deletion
}

void ToolbarRegistry::SetStandardToolbar(std::unique_ptr<StandardToolBar> tb) {
	standard_toolbar = std::move(tb);
}

void ToolbarRegistry::SetBrushToolbar(std::unique_ptr<BrushToolBar> tb) {
	brush_toolbar = std::move(tb);
}

void ToolbarRegistry::SetPositionToolbar(std::unique_ptr<PositionToolBar> tb) {
	position_toolbar = std::move(tb);
}

void ToolbarRegistry::SetSizeToolbar(std::unique_ptr<SizeToolBar> tb) {
	size_toolbar = std::move(tb);
}

void ToolbarRegistry::SetLightToolbar(std::unique_ptr<LightToolBar> tb) {
	light_toolbar = std::move(tb);
}

void ToolbarRegistry::UpdateAll() {
	if (standard_toolbar) {
		standard_toolbar->Update();
	}
	if (brush_toolbar) {
		brush_toolbar->Update();
	}
	if (position_toolbar) {
		position_toolbar->Update();
	}
	if (size_toolbar) {
		size_toolbar->Update();
	}
	// Light toolbar doesn't have an update method in original code? Checking...
	// The original code called update on it? No, it didn't call update on LightToolbar in UpdateButtons.
	// Ah, wait, UpdateButtons called standard, brush, position, size.
	// But UpdateBrushButtons called brush and then aui manager update.
}

wxAuiPaneInfo& ToolbarRegistry::GetPane(ToolBarID id, wxAuiManager* manager) {
	if (!manager) {
		return wxAuiNullPaneInfo;
	}

	switch (id) {
		case TOOLBAR_STANDARD:
			return manager->GetPane(StandardToolBar::PANE_NAME);
		case TOOLBAR_BRUSHES:
			return manager->GetPane(BrushToolBar::PANE_NAME);
		case TOOLBAR_POSITION:
			return manager->GetPane(PositionToolBar::PANE_NAME);
		case TOOLBAR_SIZES:
			return manager->GetPane(SizeToolBar::PANE_NAME);
		case TOOLBAR_LIGHT:
			return manager->GetPane(LightToolBar::PANE_NAME);
		default:
			return wxAuiNullPaneInfo;
	}
}
