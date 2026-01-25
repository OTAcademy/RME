#include "app/main.h"
#include "ui/managers/search_manager.h"
#include "ui/gui.h"
#include "ui/result_window.h"

SearchManager g_search;

SearchManager::SearchManager() :
	search_result_window(nullptr) {
}

SearchManager::~SearchManager() {
	// search_result_window is managed by AUI, it'll be deleted when root is deleted?
	// Actually GUI doesn't delete it in dtor.
}

void SearchManager::HideSearchWindow() {
	if (search_result_window) {
		g_gui.aui_manager->GetPane(search_result_window).Show(false);
		g_gui.aui_manager->Update();
	}
}

SearchResultWindow* SearchManager::ShowSearchWindow() {
	if (search_result_window == nullptr) {
		search_result_window = newd SearchResultWindow(g_gui.root);
		g_gui.aui_manager->AddPane(search_result_window, wxAuiPaneInfo().Caption("Search Results"));
	} else {
		g_gui.aui_manager->GetPane(search_result_window).Show();
	}
	g_gui.aui_manager->Update();
	return search_result_window;
}
