#include "ui/menubar/search_handler.h"
#include "app/definitions.h"

#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "ui/find_item_window.h"
#include "ui/result_window.h"
#include "ui/map_window.h"
#include "app/preferences.h"
#include "editor/editor.h"
#include "editor/operations/search_operations.h"
#include "editor/operations/clean_operations.h"
#include "map/map.h"
#include "editor/action_queue.h"

SearchHandler::SearchHandler(MainFrame* frame) :
	frame(frame) {
}

void SearchHandler::OnSearchForItem(wxCommandEvent& WXUNUSED(event)) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	FindItemDialog dialog(frame, "Search for Item");
	dialog.setSearchMode((FindItemDialog::SearchMode)g_settings.getInteger(Config::FIND_ITEM_MODE));
	if (dialog.ShowModal() == wxID_OK) {
		EditorOperations::ItemSearcher finder(dialog.getResultID(), (uint32_t)g_settings.getInteger(Config::REPLACE_SIZE));
		g_gui.CreateLoadBar("Searching map...");

		foreach_ItemOnMap(g_gui.GetCurrentMap(), finder, false);
		std::vector<std::pair<Tile*, Item*>>& result = finder.result;

		g_gui.DestroyLoadBar();

		if (finder.limitReached()) {
			wxString msg;
			msg << "The configured limit has been reached. Only " << finder.maxCount << " results will be displayed.";
			DialogUtil::PopupDialog("Notice", msg, wxOK);
		}

		SearchResultWindow* window = g_gui.ShowSearchWindow();
		window->Clear();
		for (std::vector<std::pair<Tile*, Item*>>::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
			Tile* tile = iter->first;
			Item* item = iter->second;
			window->AddPosition(wxstr(std::string(item->getName())), tile->getPosition());
		}

		g_settings.setInteger(Config::FIND_ITEM_MODE, (int)dialog.getSearchMode());
	}
	dialog.Destroy();
}

void SearchHandler::OnReplaceItems(wxCommandEvent& WXUNUSED(event)) {
	if (!g_version.IsVersionLoaded()) {
		return;
	}

	if (MapTab* tab = g_gui.GetCurrentMapTab()) {
		if (MapWindow* window = tab->GetView()) {
			window->ShowReplaceItemsDialog(false);
		}
	}
}

void SearchHandler::OnSearchForStuffOnMap(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(true, true, true, true);
}

void SearchHandler::OnSearchForUniqueOnMap(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(true, false, false, false);
}

void SearchHandler::OnSearchForActionOnMap(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, true, false, false);
}

void SearchHandler::OnSearchForContainerOnMap(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, false, true, false);
}

void SearchHandler::OnSearchForWriteableOnMap(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, false, false, true);
}

void SearchHandler::OnSearchForStuffOnSelection(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(true, true, true, true, true);
}

void SearchHandler::OnSearchForUniqueOnSelection(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(true, false, false, false, true);
}

void SearchHandler::OnSearchForActionOnSelection(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, true, false, false, true);
}

void SearchHandler::OnSearchForContainerOnSelection(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, false, true, false, true);
}

void SearchHandler::OnSearchForWriteableOnSelection(wxCommandEvent& WXUNUSED(event)) {
	SearchItems(false, false, false, true, true);
}

void SearchHandler::OnSearchForItemOnSelection(wxCommandEvent& WXUNUSED(event)) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	FindItemDialog dialog(frame, "Search on Selection");
	dialog.setSearchMode((FindItemDialog::SearchMode)g_settings.getInteger(Config::FIND_ITEM_MODE));
	if (dialog.ShowModal() == wxID_OK) {
		EditorOperations::ItemSearcher finder(dialog.getResultID(), (uint32_t)g_settings.getInteger(Config::REPLACE_SIZE));
		g_gui.CreateLoadBar("Searching on selected area...");

		foreach_ItemOnMap(g_gui.GetCurrentMap(), finder, true);
		std::vector<std::pair<Tile*, Item*>>& result = finder.result;

		g_gui.DestroyLoadBar();

		if (finder.limitReached()) {
			wxString msg;
			msg << "The configured limit has been reached. Only " << finder.maxCount << " results will be displayed.";
			DialogUtil::PopupDialog("Notice", msg, wxOK);
		}

		SearchResultWindow* window = g_gui.ShowSearchWindow();
		window->Clear();
		for (std::vector<std::pair<Tile*, Item*>>::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
			Tile* tile = iter->first;
			Item* item = iter->second;
			window->AddPosition(wxstr(std::string(item->getName())), tile->getPosition());
		}

		g_settings.setInteger(Config::FIND_ITEM_MODE, (int)dialog.getSearchMode());
	}

	dialog.Destroy();
}

void SearchHandler::OnReplaceItemsOnSelection(wxCommandEvent& WXUNUSED(event)) {
	if (!g_version.IsVersionLoaded()) {
		return;
	}

	if (MapTab* tab = g_gui.GetCurrentMapTab()) {
		if (MapWindow* window = tab->GetView()) {
			window->ShowReplaceItemsDialog(true);
		}
	}
}

void SearchHandler::OnRemoveItemOnSelection(wxCommandEvent& WXUNUSED(event)) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	FindItemDialog dialog(frame, "Remove Item on Selection");
	if (dialog.ShowModal() == wxID_OK) {
		g_gui.GetCurrentEditor()->actionQueue->clear();
		g_gui.CreateLoadBar("Searching item on selection to remove...");
		EditorOperations::RemoveItemCondition condition(dialog.getResultID());
		int64_t count = RemoveItemOnMap(g_gui.GetCurrentMap(), condition, true);
		g_gui.DestroyLoadBar();

		wxString msg;
		msg << count << " items removed.";
		DialogUtil::PopupDialog("Remove Item", msg, wxOK);
		g_gui.GetCurrentMap().doChange();
		g_gui.RefreshView();
	}
	dialog.Destroy();
}

struct SearchResult {
	Tile* tile;
	Item* item;
	std::string description;
};

void SearchHandler::SearchItems(bool unique, bool action, bool container, bool writable, bool onSelection /* = false*/) {
	if (!unique && !action && !container && !writable) {
		return;
	}

	if (!g_gui.IsEditorOpen()) {
		return;
	}

	if (onSelection) {
		g_gui.CreateLoadBar("Searching on selected area...");
	} else {
		g_gui.CreateLoadBar("Searching on map...");
	}

	// Use the MapSearcher from EditorOperations
	EditorOperations::MapSearcher finder;
	finder.search_unique = unique;
	finder.search_action = action;
	finder.search_container = container;
	finder.search_writeable = writable;

	foreach_ItemOnMap(g_gui.GetCurrentMap(), finder, onSelection);
	finder.sort();

	std::vector<SearchResult> found;
	for (const auto& pair : finder.found) {
		SearchResult res;
		res.tile = pair.first;
		res.item = pair.second;
		res.description = finder.desc(res.item).ToStdString();
		found.push_back(res);
	}

	g_gui.DestroyLoadBar();

	SearchResultWindow* result = g_gui.ShowSearchWindow();
	result->Clear();
	for (const auto& res : found) {
		result->AddPosition(wxstr(res.description), res.tile->getPosition());
	}
}
