#include <iostream>
#include "ingame_preview/ingame_preview_manager.h"
#include "ingame_preview/ingame_preview_window.h"
#include "util/image_manager.h"

#include "ui/gui.h"
#include "editor/editor.h"
#include <wx/aui/aui.h>

IngamePreview::IngamePreviewManager g_preview;

namespace IngamePreview {

	IngamePreviewManager::IngamePreviewManager() {
	}

	IngamePreviewManager::~IngamePreviewManager() {
		if (window && g_gui.aui_manager) {
			g_gui.aui_manager->DetachPane(window.get());
		}
	}

	void IngamePreviewManager::Create() {

		if (!g_gui.aui_manager) {
			return;
		}

		if (window) {
			g_gui.aui_manager->GetPane(window.get()).Show(true);
		} else {
			window = std::make_unique<IngamePreviewWindow>(g_gui.root);
			g_gui.aui_manager->AddPane(window.get(), wxAuiPaneInfo().Name("IngamePreview").Caption("In-game Preview").Right().Dockable(true).FloatingSize(400, 300).MinSize(200, 150).Hide());

			g_gui.aui_manager->GetPane(window.get()).Show(true);
		}
		g_gui.aui_manager->Update();
	}

	void IngamePreviewManager::Hide() {
		if (!g_gui.aui_manager) {
			return;
		}

		if (window) {
			g_gui.aui_manager->GetPane(window.get()).Show(false);
			g_gui.aui_manager->Update();
		}
	}

	void IngamePreviewManager::Destroy() {
		if (window) {
			if (g_gui.aui_manager) {
				g_gui.aui_manager->DetachPane(window.get());
				g_gui.aui_manager->Update();
			}
			// window->Destroy(); // wxWindow::Destroy calls delete this; which might double free with unique_ptr
			// However, if we detach from wx parent or AUI first...
			// The style guide says use unique_ptr. Unique_ptr destructor calls delete.
			// delete calls ~wxWindow(), which removes from parent. This is safe.
			// Calling Destroy() queues deletion? No, for normal windows it's effectively delete.
			// We just reset() the unique_ptr.
			window.reset();
		}
	}

	void IngamePreviewManager::Update() {
		if (window && IsVisible()) {
			window->UpdateState();
		}
	}

	bool IngamePreviewManager::IsVisible() const {
		if (window && g_gui.aui_manager) {
			const wxAuiPaneInfo& pi = g_gui.aui_manager->GetPane(window.get());
			return pi.IsShown();
		}
		return false;
	}

} // namespace IngamePreview
