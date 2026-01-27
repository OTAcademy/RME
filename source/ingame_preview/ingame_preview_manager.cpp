#include <iostream>
#include "ingame_preview/ingame_preview_manager.h"
#include "ingame_preview/ingame_preview_window.h"
#include "ui/gui.h"
#include "editor/editor.h"
#include <wx/aui/aui.h>

IngamePreview::IngamePreviewManager g_preview;

namespace IngamePreview {

	IngamePreviewManager::IngamePreviewManager() :
		window(nullptr) {
	}

	IngamePreviewManager::~IngamePreviewManager() {
		if (window && g_gui.aui_manager) {
			g_gui.aui_manager->DetachPane(window);
		}
	}

	void IngamePreviewManager::Create() {
		if (!g_gui.IsAnyEditorOpen()) {
			return;
		}

		if (window) {
			g_gui.aui_manager->GetPane(window).Show(true);
		} else {
			window = new IngamePreviewWindow(g_gui.root, *g_gui.GetCurrentEditor());
			g_gui.aui_manager->AddPane(window, wxAuiPaneInfo().Name("IngamePreview").Caption("In-game Preview").Right().Dockable(true).FloatingSize(400, 300).MinSize(200, 150).Hide());

			g_gui.aui_manager->GetPane(window).Show(true);
		}
		g_gui.aui_manager->Update();
	}

	void IngamePreviewManager::Hide() {
		if (window) {
			g_gui.aui_manager->GetPane(window).Show(false);
			g_gui.aui_manager->Update();
		}
	}

	void IngamePreviewManager::Destroy() {
		if (window) {
			g_gui.aui_manager->DetachPane(window);
			g_gui.aui_manager->Update();
			window->Destroy();
			window = nullptr;
		}
	}

	void IngamePreviewManager::Update() {
		if (window && IsVisible()) {
			window->UpdateState();
		}
	}

	bool IngamePreviewManager::IsVisible() const {
		if (window && g_gui.aui_manager) {
			const wxAuiPaneInfo& pi = g_gui.aui_manager->GetPane(window);
			return pi.IsShown();
		}
		return false;
	}

} // namespace IngamePreview
