//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "ui/main_menubar.h"
#include "ui/dialog_util.h"
#include "ui/gui.h"
#include "ui/tool_options_window.h"

#include "map/map_statistics.h"
#include "map/map_search.h"
#include "map/map_search.h"
#include "ui/managers/recent_files_manager.h"
#include "ui/map/towns_window.h"
#include "ui/map/map_properties_window.h"

#include "ui/about_window.h"
#include "ui/dat_debug_view.h"
#include "ui/menubar_loader.h"
#include "ui/map_statistics_dialog.h"
#include "ui/live_dialogs.h"

#include "ui/extension_window.h"
#include "ui/find_item_window.h"
#include "app/preferences.h"
#include "ui/result_window.h"

#include "ui/menubar/search_handler.h"
#include "ui/menubar/view_settings_handler.h"
#include "ui/menubar/map_actions_handler.h"
#include "ui/menubar/file_menu_handler.h"
#include "ui/menubar/navigation_menu_handler.h"
#include "ui/menubar/palette_menu_handler.h"
#include "ui/menubar/menubar_action_manager.h"
#include "ingame_preview/ingame_preview_manager.h"

#include <wx/chartype.h>

#include "editor/editor.h"
#include "game/materials.h"
#include "live/live_client.h"
#include "editor/action_queue.h"

#include "editor/operations/search_operations.h"
#include "editor/operations/clean_operations.h"

BEGIN_EVENT_TABLE(MainMenuBar, wxEvtHandler)
END_EVENT_TABLE()

MainMenuBar::MainMenuBar(MainFrame* frame) :
	frame(frame) {
	using namespace MenuBar;
	checking_programmaticly = false;

	searchHandler = new SearchHandler(frame);
	viewSettingsHandler = new ViewSettingsHandler(this);
	mapActionsHandler = new MapActionsHandler(frame);
	fileMenuHandler = new FileMenuHandler(frame, this);
	navigationMenuHandler = new NavigationMenuHandler(frame, this);
	paletteMenuHandler = new PaletteMenuHandler(frame, this);

	MenuBarActionManager::RegisterActions(this, actions);

	// A deleter, this way the frame does not need

	// A deleter, this way the frame does not need
	// to bother deleting us.
	class CustomMenuBar : public wxMenuBar {
	public:
		CustomMenuBar(MainMenuBar* mb) :
			mb(mb) { }
		~CustomMenuBar() {
			delete mb;
		}

	private:
		MainMenuBar* mb;
	};

	menubar = newd CustomMenuBar(this);
	frame->SetMenuBar(menubar);

	// Tie all events to this handler!

	for (std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		frame->Connect(MAIN_FRAME_MENU + ai->second->id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)(wxEventFunction)(ai->second->handler), nullptr, this);
	}
	for (size_t i = 0; i < 10; ++i) {
		frame->Connect(recentFilesManager.GetBaseId() + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainMenuBar::OnOpenRecent), nullptr, this);
	}
}

MainMenuBar::~MainMenuBar() {
	// Don't need to delete menubar, it's owned by the frame

	for (std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		delete ai->second;
	}

	delete searchHandler;
	delete viewSettingsHandler;
	delete mapActionsHandler;
	delete fileMenuHandler;
	delete navigationMenuHandler;
	delete paletteMenuHandler;
}

void MainMenuBar::EnableItem(MenuBar::ActionID id, bool enable) {
	std::map<MenuBar::ActionID, std::list<wxMenuItem*>>::iterator fi = items.find(id);
	if (fi == items.end()) {
		return;
	}

	std::list<wxMenuItem*>& li = fi->second;

	for (std::list<wxMenuItem*>::iterator i = li.begin(); i != li.end(); ++i) {
		(*i)->Enable(enable);
	}
}

void MainMenuBar::CheckItem(MenuBar::ActionID id, bool enable) {
	std::map<MenuBar::ActionID, std::list<wxMenuItem*>>::iterator fi = items.find(id);
	if (fi == items.end()) {
		return;
	}

	std::list<wxMenuItem*>& li = fi->second;

	checking_programmaticly = true;
	for (std::list<wxMenuItem*>::iterator i = li.begin(); i != li.end(); ++i) {
		(*i)->Check(enable);
	}
	checking_programmaticly = false;
}

bool MainMenuBar::IsItemChecked(MenuBar::ActionID id) const {
	std::map<MenuBar::ActionID, std::list<wxMenuItem*>>::const_iterator fi = items.find(id);
	if (fi == items.end()) {
		return false;
	}

	const std::list<wxMenuItem*>& li = fi->second;

	for (std::list<wxMenuItem*>::const_iterator i = li.begin(); i != li.end(); ++i) {
		if ((*i)->IsChecked()) {
			return true;
		}
	}

	return false;
}

void MainMenuBar::Update() {
	using namespace MenuBar;
	// This updates all buttons and sets them to proper enabled/disabled state

	MenuBarActionManager::UpdateState(this);
}

void MainMenuBar::LoadValues() {
	viewSettingsHandler->LoadValues();
}

void MainMenuBar::UpdateFloorMenu() {
	// this will have to be changed if you want to have more floors
	// see MAKE_ACTION(FLOOR_0, wxITEM_RADIO, OnChangeFloor);
	if (MAP_MAX_LAYER < 16) {
		if (g_gui.IsEditorOpen()) {
			for (int i = 0; i < MAP_LAYERS; ++i) {
				CheckItem(MenuBar::ActionID(MenuBar::FLOOR_0 + i), false);
			}
			CheckItem(MenuBar::ActionID(MenuBar::FLOOR_0 + g_gui.GetCurrentFloor()), true);
		}
	}
}

bool MainMenuBar::Load(const FileName& path, wxArrayString& warnings, wxString& error) {
	if (MenuBarLoader::Load(path, menubar, items, actions, recentFilesManager, warnings, error)) {
		Update();
		LoadValues();
		return true;
	}
	return false;
}

// LoadItem moved to MenuBarLoader

// LoadItem moved to MenuBarLoader

void MainMenuBar::OnNew(wxCommandEvent& event) {
	fileMenuHandler->OnNew(event);
}

void MainMenuBar::OnGenerateMap(wxCommandEvent& event) {
	fileMenuHandler->OnGenerateMap(event);
}

void MainMenuBar::OnOpenRecent(wxCommandEvent& event) {
	FileName fn(recentFilesManager.GetFile(event.GetId() - recentFilesManager.GetBaseId()));
	frame->LoadMap(fn);
}

void MainMenuBar::OnOpen(wxCommandEvent& event) {
	fileMenuHandler->OnOpen(event);
}

void MainMenuBar::OnClose(wxCommandEvent& event) {
	fileMenuHandler->OnClose(event);
}

void MainMenuBar::OnSave(wxCommandEvent& event) {
	fileMenuHandler->OnSave(event);
}

void MainMenuBar::OnSaveAs(wxCommandEvent& event) {
	fileMenuHandler->OnSaveAs(event);
}

void MainMenuBar::OnPreferences(wxCommandEvent& event) {
	fileMenuHandler->OnPreferences(event);
}

void MainMenuBar::OnQuit(wxCommandEvent& event) {
	fileMenuHandler->OnQuit(event);
}

void MainMenuBar::OnImportMap(wxCommandEvent& event) {
	fileMenuHandler->OnImportMap(event);
}

void MainMenuBar::OnImportMonsterData(wxCommandEvent& event) {
	fileMenuHandler->OnImportMonsterData(event);
}

void MainMenuBar::OnImportMinimap(wxCommandEvent& event) {
	fileMenuHandler->OnImportMinimap(event);
}

void MainMenuBar::OnExportTilesets(wxCommandEvent& event) {
	fileMenuHandler->OnExportTilesets(event);
}

void MainMenuBar::OnDebugViewDat(wxCommandEvent& event) {
	wxDialog dlg(frame, wxID_ANY, "Debug .dat file", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	new DatDebugView(&dlg);
	dlg.ShowModal();
}

void MainMenuBar::OnReloadDataFiles(wxCommandEvent& event) {
	fileMenuHandler->OnReloadDataFiles(event);
}

void MainMenuBar::OnListExtensions(wxCommandEvent& event) {
	fileMenuHandler->OnListExtensions(event);
}

void MainMenuBar::OnGotoWebsite(wxCommandEvent& event) {
	fileMenuHandler->OnGotoWebsite(event);
}

void MainMenuBar::OnAbout(wxCommandEvent& event) {
	fileMenuHandler->OnAbout(event);
}

void MainMenuBar::OnUndo(wxCommandEvent& WXUNUSED(event)) {
	g_gui.DoUndo();
}

void MainMenuBar::OnRedo(wxCommandEvent& WXUNUSED(event)) {
	g_gui.DoRedo();
}

void MainMenuBar::OnSearchForItem(wxCommandEvent& event) {
	searchHandler->OnSearchForItem(event);
}

void MainMenuBar::OnReplaceItems(wxCommandEvent& event) {
	searchHandler->OnReplaceItems(event);
}

void MainMenuBar::OnSearchForStuffOnMap(wxCommandEvent& event) {
	searchHandler->OnSearchForStuffOnMap(event);
}

void MainMenuBar::OnSearchForUniqueOnMap(wxCommandEvent& event) {
	searchHandler->OnSearchForUniqueOnMap(event);
}

void MainMenuBar::OnSearchForActionOnMap(wxCommandEvent& event) {
	searchHandler->OnSearchForActionOnMap(event);
}

void MainMenuBar::OnSearchForContainerOnMap(wxCommandEvent& event) {
	searchHandler->OnSearchForContainerOnMap(event);
}

void MainMenuBar::OnSearchForWriteableOnMap(wxCommandEvent& event) {
	searchHandler->OnSearchForWriteableOnMap(event);
}

void MainMenuBar::OnSearchForStuffOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForStuffOnSelection(event);
}

void MainMenuBar::OnSearchForUniqueOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForUniqueOnSelection(event);
}

void MainMenuBar::OnSearchForActionOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForActionOnSelection(event);
}

void MainMenuBar::OnSearchForContainerOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForContainerOnSelection(event);
}

void MainMenuBar::OnSearchForWriteableOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForWriteableOnSelection(event);
}

void MainMenuBar::OnSearchForItemOnSelection(wxCommandEvent& event) {
	searchHandler->OnSearchForItemOnSelection(event);
}

void MainMenuBar::OnReplaceItemsOnSelection(wxCommandEvent& event) {
	searchHandler->OnReplaceItemsOnSelection(event);
}

void MainMenuBar::OnRemoveItemOnSelection(wxCommandEvent& event) {
	searchHandler->OnRemoveItemOnSelection(event);
}

void MainMenuBar::OnSelectionTypeChange(wxCommandEvent& event) {
	viewSettingsHandler->OnSelectionTypeChange(event);
}

void MainMenuBar::OnCopy(wxCommandEvent& WXUNUSED(event)) {
	g_gui.DoCopy();
}

void MainMenuBar::OnCut(wxCommandEvent& WXUNUSED(event)) {
	g_gui.DoCut();
}

void MainMenuBar::OnPaste(wxCommandEvent& WXUNUSED(event)) {
	g_gui.PreparePaste();
}

void MainMenuBar::OnToggleAutomagic(wxCommandEvent& event) {
	viewSettingsHandler->OnToggleAutomagic(event);
}

void MainMenuBar::OnBorderizeSelection(wxCommandEvent& event) {
	mapActionsHandler->OnBorderizeSelection(event);
}

void MainMenuBar::OnBorderizeMap(wxCommandEvent& event) {
	mapActionsHandler->OnBorderizeMap(event);
}

void MainMenuBar::OnRandomizeSelection(wxCommandEvent& event) {
	mapActionsHandler->OnRandomizeSelection(event);
}

void MainMenuBar::OnRandomizeMap(wxCommandEvent& event) {
	mapActionsHandler->OnRandomizeMap(event);
}

void MainMenuBar::OnJumpToBrush(wxCommandEvent& event) {
	navigationMenuHandler->OnJumpToBrush(event);
}

void MainMenuBar::OnJumpToItemBrush(wxCommandEvent& event) {
	navigationMenuHandler->OnJumpToItemBrush(event);
}

void MainMenuBar::OnGotoPreviousPosition(wxCommandEvent& event) {
	navigationMenuHandler->OnGotoPreviousPosition(event);
}

void MainMenuBar::OnGotoPosition(wxCommandEvent& event) {
	navigationMenuHandler->OnGotoPosition(event);
}

void MainMenuBar::OnMapRemoveItems(wxCommandEvent& event) {
	mapActionsHandler->OnMapRemoveItems(event);
}

void MainMenuBar::OnMapRemoveCorpses(wxCommandEvent& event) {
	mapActionsHandler->OnMapRemoveCorpses(event);
}

void MainMenuBar::OnMapRemoveUnreachable(wxCommandEvent& event) {
	mapActionsHandler->OnMapRemoveUnreachable(event);
}

void MainMenuBar::OnClearHouseTiles(wxCommandEvent& event) {
	mapActionsHandler->OnClearHouseTiles(event);
}

void MainMenuBar::OnClearModifiedState(wxCommandEvent& event) {
	mapActionsHandler->OnClearModifiedState(event);
}

void MainMenuBar::OnMapCleanHouseItems(wxCommandEvent& event) {
	mapActionsHandler->OnMapCleanHouseItems(event);
}

void MainMenuBar::OnMapEditTowns(wxCommandEvent& WXUNUSED(event)) {
	if (g_gui.GetCurrentEditor()) {
		wxDialog* town_dialog = newd EditTownsDialog(frame, *g_gui.GetCurrentEditor());
		town_dialog->ShowModal();
		town_dialog->Destroy();
	}
}

void MainMenuBar::OnMapEditItems(wxCommandEvent& WXUNUSED(event)) {
	;
}

void MainMenuBar::OnMapEditMonsters(wxCommandEvent& WXUNUSED(event)) {
	;
}

void MainMenuBar::OnMapStatistics(wxCommandEvent& WXUNUSED(event)) {
	MapStatisticsDialog::Show(frame);
}

void MainMenuBar::OnMapCleanup(wxCommandEvent& event) {
	mapActionsHandler->OnMapCleanup(event);
}

void MainMenuBar::OnMapProperties(wxCommandEvent& WXUNUSED(event)) {
	wxDialog* properties = newd MapPropertiesWindow(
		frame,
		static_cast<MapTab*>(g_gui.GetCurrentTab()),
		*g_gui.GetCurrentEditor()
	);

	if (properties->ShowModal() == 0) {
		// FAIL!
		g_gui.CloseAllEditors();
	}
	properties->Destroy();
}

void MainMenuBar::OnToolbars(wxCommandEvent& event) {
	viewSettingsHandler->OnToolbars(event);
}

void MainMenuBar::OnNewView(wxCommandEvent& WXUNUSED(event)) {
	g_gui.NewMapView();
}

void MainMenuBar::OnToggleFullscreen(wxCommandEvent& WXUNUSED(event)) {
	if (frame->IsFullScreen()) {
		frame->ShowFullScreen(false);
	} else {
		frame->ShowFullScreen(true, wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
	}
}

void MainMenuBar::OnTakeScreenshot(wxCommandEvent& WXUNUSED(event)) {
	wxString path = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	if (!path.empty() && (path.Last() == '/' || path.Last() == '\\')) {
		path = path + "/";
	}

	g_gui.GetCurrentMapTab()->GetView()->GetCanvas()->TakeScreenshot(
		path, wxstr(g_settings.getString(Config::SCREENSHOT_FORMAT))
	);
}

void MainMenuBar::OnZoomIn(wxCommandEvent& event) {
	navigationMenuHandler->OnZoomIn(event);
}

void MainMenuBar::OnZoomOut(wxCommandEvent& event) {
	navigationMenuHandler->OnZoomOut(event);
}

void MainMenuBar::OnZoomNormal(wxCommandEvent& event) {
	navigationMenuHandler->OnZoomNormal(event);
}

void MainMenuBar::OnChangeViewSettings(wxCommandEvent& event) {
	viewSettingsHandler->OnChangeViewSettings(event);
}

void MainMenuBar::OnChangeFloor(wxCommandEvent& event) {
	navigationMenuHandler->OnChangeFloor(event);
}

void MainMenuBar::OnMinimapWindow(wxCommandEvent& event) {
	g_gui.CreateMinimap();
}

void MainMenuBar::OnToolOptionsWindow(wxCommandEvent& event) {
	if (g_gui.tool_options) {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_gui.tool_options);
		if (info.IsShown()) {
			info.Hide();
		} else {
			info.Show();
		}
		g_gui.aui_manager->Update();
	}
}

void MainMenuBar::OnIngamePreviewWindow(wxCommandEvent& event) {
	g_preview.Create();
}

void MainMenuBar::OnNewPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnNewPalette(event);
}

void MainMenuBar::OnSelectTerrainPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectTerrainPalette(event);
}

void MainMenuBar::OnSelectDoodadPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectDoodadPalette(event);
}

void MainMenuBar::OnSelectItemPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectItemPalette(event);
}

void MainMenuBar::OnSelectCollectionPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectCollectionPalette(event);
}

void MainMenuBar::OnSelectHousePalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectHousePalette(event);
}

void MainMenuBar::OnSelectCreaturePalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectCreaturePalette(event);
}

void MainMenuBar::OnSelectWaypointPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectWaypointPalette(event);
}

void MainMenuBar::OnSelectRawPalette(wxCommandEvent& event) {
	paletteMenuHandler->OnSelectRawPalette(event);
}

void MainMenuBar::OnStartLive(wxCommandEvent& event) {
	LiveDialogs::ShowHostDialog(frame, g_gui.GetCurrentEditor());
}

void MainMenuBar::OnJoinLive(wxCommandEvent& event) {
	LiveDialogs::ShowJoinDialog(frame);
	Update();
}

void MainMenuBar::OnCloseLive(wxCommandEvent& event) {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor && editor->live_manager.IsLive()) {
		g_gui.CloseLiveEditors(&editor->live_manager.GetSocket());
	}

	Update();
}
