//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/managers/minimap_manager.h"
#include "app/managers/version_manager.h"
#include "ui/gui.h"
#include "rendering/ui/minimap_window.h"
#include <wx/aui/aui.h>

MinimapManager g_minimap;

MinimapManager::MinimapManager() : minimap(nullptr) {
}

MinimapManager::~MinimapManager() {
	// Root window will destroy minimap window if it's a child.
	// But we should detach it from aui_manager if it exists.
	if (minimap && g_gui.aui_manager) {
		g_gui.aui_manager->DetachPane(minimap);
	}
}

void MinimapManager::Create() {
	if (!g_version.IsVersionLoaded()) {
		return;
	}

	if (minimap) {
		g_gui.aui_manager->GetPane(minimap).Show(true);
	} else {
		minimap = newd MinimapWindow(g_gui.root);
		minimap->Show(true);
		g_gui.aui_manager->AddPane(minimap, wxAuiPaneInfo().Caption("Minimap"));
	}
	g_gui.aui_manager->Update();
}

void MinimapManager::Hide() {
	if (minimap) {
		g_gui.aui_manager->GetPane(minimap).Show(false);
		g_gui.aui_manager->Update();
	}
}

void MinimapManager::Destroy() {
	if (minimap) {
		g_gui.aui_manager->DetachPane(minimap);
		g_gui.aui_manager->Update();
		minimap->Destroy();
		minimap = nullptr;
	}
}

void MinimapManager::Update(bool immediate) {
	if (IsVisible()) {
		if (immediate) {
			minimap->Refresh();
		} else {
			minimap->DelayedUpdate();
		}
	}
}

bool MinimapManager::IsVisible() const {
	if (minimap && g_gui.aui_manager) {
		const wxAuiPaneInfo& pi = g_gui.aui_manager->GetPane(minimap);
		if (pi.IsShown()) {
			return true;
		}
	}
	return false;
}
