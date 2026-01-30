//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/managers/loading_manager.h"

#include "ui/gui.h"
#include "editor/editor.h"
#include "ui/map_tab.h"
#include "live/live_server.h"

LoadingManager g_loading;

LoadingManager::LoadingManager() :
	progressBar(nullptr),
	progressFrom(0),
	progressTo(100),
	currentProgress(-1) {
}

LoadingManager::~LoadingManager() {
	DestroyLoadBar();
}

void LoadingManager::CreateLoadBar(wxString message, bool canCancel) {
	progressText = message;

	progressFrom = 0;
	progressTo = 100;
	currentProgress = -1;

	progressBar = newd wxGenericProgressDialog("Loading", progressText + " (0%)", 100, g_gui.root, wxPD_APP_MODAL | wxPD_SMOOTH | (canCancel ? wxPD_CAN_ABORT : 0));
	progressBar->Show(true);

	if (g_gui.tabbook) {
		for (int idx = 0; idx < g_gui.tabbook->GetTabCount(); ++idx) {
			auto* mt = dynamic_cast<MapTab*>(g_gui.tabbook->GetTab(idx));
			if (mt && mt->GetEditor()->live_manager.IsServer()) {
				mt->GetEditor()->live_manager.GetServer()->startOperation(progressText);
			}
		}
	}
	progressBar->Update(0);
}

void LoadingManager::SetLoadScale(int32_t from, int32_t to) {
	progressFrom = from;
	progressTo = to;
}

bool LoadingManager::SetLoadDone(int32_t done, const wxString& newMessage) {
	if (done == 100) {
		DestroyLoadBar();
		return true;
	} else if (done == currentProgress) {
		return true;
	}

	if (!newMessage.empty()) {
		progressText = newMessage;
	}

	int32_t newProgress = progressFrom + static_cast<int32_t>((done / 100.f) * (progressTo - progressFrom));
	newProgress = std::max<int32_t>(0, std::min<int32_t>(100, newProgress));

	bool skip = false;
	if (progressBar) {
		progressBar->Update(
			newProgress,
			wxString::Format("%s (%d%%)", progressText, newProgress),
			&skip
		);
		currentProgress = newProgress;
	}

	if (g_gui.tabbook) {
		for (int32_t index = 0; index < g_gui.tabbook->GetTabCount(); ++index) {
			auto* mapTab = dynamic_cast<MapTab*>(g_gui.tabbook->GetTab(index));
			if (mapTab && mapTab->GetEditor()) {
				LiveServer* server = mapTab->GetEditor()->live_manager.GetServer();
				if (server) {
					server->updateOperation(newProgress);
				}
			}
		}
	}

	return skip;
}

void LoadingManager::DestroyLoadBar() {
	if (progressBar) {
		progressBar->Show(false);
		currentProgress = -1;

		progressBar->Destroy();
		progressBar = nullptr;

		if (g_gui.root && g_gui.root->IsActive()) {
			g_gui.root->Raise();
		} else if (g_gui.root) {
			g_gui.root->RequestUserAttention();
		}
	}
}
