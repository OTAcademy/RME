//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "editor/managers/editor_manager.h"
#include "ui/gui.h"
#include "ui/map_tab.h"
#include "ui/main_menubar.h"
#include "palette/palette_window.h"
#include "editor/editor.h"
#include "editor/editor_factory.h"
#include "editor/action_queue.h"
#include "ui/dialog_util.h"
#include "ui/welcome_dialog.h"
#include "app/application.h"
#include "app/managers/version_manager.h"
#include "ui/managers/status_manager.h"
#include "palette/managers/palette_manager.h"
#include "brushes/managers/brush_manager.h"
#include "live/live_tab.h"
#include "live/live_client.h"
#include "io/iomap_otbm.h"

#include <set>
#include <sstream>

EditorManager g_editors;

EditorManager::EditorManager() {
}

EditorManager::~EditorManager() {
}

EditorTab* EditorManager::GetCurrentTab() {
	return g_gui.tabbook->GetCurrentTab();
}

EditorTab* EditorManager::GetTab(int idx) {
	return g_gui.tabbook->GetTab(idx);
}

int EditorManager::GetTabCount() const {
	return g_gui.tabbook->GetTabCount();
}

bool EditorManager::IsAnyEditorOpen() const {
	return g_gui.tabbook && g_gui.tabbook->GetTabCount() > 0;
}

bool EditorManager::IsEditorOpen() const {
	return g_gui.tabbook != nullptr && GetCurrentMapTab();
}

void EditorManager::CloseCurrentEditor() {
	spdlog::info("EditorManager::CloseCurrentEditor - Closing current tab");
	g_palettes.RefreshPalettes();
	g_gui.tabbook->DeleteTab(g_gui.tabbook->GetSelection());
	g_gui.root->UpdateMenubar();
	spdlog::info("EditorManager::CloseCurrentEditor - Closed current tab");
}

Editor* EditorManager::GetCurrentEditor() {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		return mapTab->GetEditor();
	}
	return nullptr;
}

MapTab* EditorManager::GetCurrentMapTab() const {
	if (g_gui.tabbook && g_gui.tabbook->GetTabCount() > 0) {
		EditorTab* editorTab = g_gui.tabbook->GetCurrentTab();
		auto* mapTab = dynamic_cast<MapTab*>(editorTab);
		return mapTab;
	}
	return nullptr;
}

void EditorManager::CycleTab(bool forward) {
	g_gui.tabbook->CycleTab(forward);
}

bool EditorManager::CloseLiveEditors(LiveSocket* sock) {
	for (int i = 0; i < g_gui.tabbook->GetTabCount(); ++i) {
		auto* mapTab = dynamic_cast<MapTab*>(g_gui.tabbook->GetTab(i));
		if (mapTab) {
			Editor* editor = mapTab->GetEditor();
			if (editor->live_manager.GetClient() == sock) {
				g_gui.tabbook->DeleteTab(i--);
			}
		}
		auto* liveLogTab = dynamic_cast<LiveLogTab*>(g_gui.tabbook->GetTab(i));
		if (liveLogTab) {
			if (liveLogTab->GetSocket() == sock) {
				liveLogTab->Disconnect();
				g_gui.tabbook->DeleteTab(i--);
			}
		}
	}
	g_gui.root->UpdateMenubar();
	return true;
}

bool EditorManager::CloseAllEditors() {
	spdlog::info("EditorManager::CloseAllEditors - Closing all tabs");
	for (int i = 0; i < g_gui.tabbook->GetTabCount(); ++i) {
		auto* mapTab = dynamic_cast<MapTab*>(g_gui.tabbook->GetTab(i));
		if (mapTab) {
			if (mapTab->IsUniqueReference() && mapTab->GetMap() && mapTab->GetMap()->hasChanged()) {
				g_gui.tabbook->SetFocusedTab(i);
				if (!g_gui.root->DoQuerySave(false)) {
					spdlog::info("EditorManager::CloseAllEditors - Cancelled by user");
					return false;
				} else {
					g_palettes.RefreshPalettes();
					g_gui.tabbook->DeleteTab(i--);
				}
			} else {
				g_gui.tabbook->DeleteTab(i--);
			}
		}
	}
	if (g_gui.root) {
		g_gui.root->UpdateMenubar();
	}
	spdlog::info("EditorManager::CloseAllEditors - All tabs closed");
	return true;
}

void EditorManager::NewMapView() {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		auto* newMapTab = newd MapTab(mapTab);
		newMapTab->OnSwitchEditorMode(g_gui.mode);

		g_status.SetStatusText("Created new view");
		g_status.UpdateTitle();
		g_palettes.RefreshPalettes();
		g_gui.root->UpdateMenubar();
		g_gui.root->Refresh();
	}
}

Map& EditorManager::GetCurrentMap() {
	Editor* editor = GetCurrentEditor();
	ASSERT(editor);
	return editor->map;
}

int EditorManager::GetOpenMapCount() {
	std::set<Map*> open_maps;

	for (int i = 0; i < g_gui.tabbook->GetTabCount(); ++i) {
		auto* tab = dynamic_cast<MapTab*>(g_gui.tabbook->GetTab(i));
		if (tab) {
			open_maps.insert(tab->GetMap());
		}
	}

	return static_cast<int>(open_maps.size());
}

bool EditorManager::ShouldSave() {
	const Map& map = GetCurrentMap();
	if (map.hasChanged()) {
		if (map.getTileCount() == 0) {
			Editor* editor = GetCurrentEditor();
			ASSERT(editor);
			return editor->actionQueue->canUndo();
		}
		return true;
	}
	return false;
}

void EditorManager::SaveCurrentMap(FileName fileName, bool showdialog) {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		Editor* editor = mapTab->GetEditor();
		if (editor) {
			g_status.SetStatusText("Saving map...");
			if (g_gui.root) {
				g_gui.root->Update();
			}

			EditorPersistence::saveMap(*editor, fileName, showdialog);

			if (!editor->map.hasChanged()) {
				g_status.SetStatusText("Map saved successfully.");
			}

			const std::string& path = editor->map.getFilename();
			const Position& position = mapTab->GetScreenCenterPosition();
			std::ostringstream stream;
			stream << position;
			g_settings.setString(Config::RECENT_EDITED_MAP_PATH, path);
			g_settings.setString(Config::RECENT_EDITED_MAP_POSITION, stream.str());
		}
	}

	g_status.UpdateTitle();
	g_gui.root->UpdateMenubar();
	g_gui.root->Refresh();
}

bool EditorManager::NewMap() {
	spdlog::info("EditorManager::NewMap - Creating new map");
	g_gui.FinishWelcomeDialog();

	std::unique_ptr<Editor> editor;
	try {
		editor = EditorFactory::CreateEmpty(g_gui.copybuffer);
	} catch (std::runtime_error& e) {
		DialogUtil::PopupDialog(g_gui.root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	auto* mapTab = newd MapTab(g_gui.tabbook.get(), editor.release());
	mapTab->OnSwitchEditorMode(g_gui.mode);
	mapTab->GetMap()->clearChanges();

	g_status.SetStatusText("Created new map");
	g_status.UpdateTitle();
	g_palettes.RefreshPalettes();
	g_gui.root->UpdateMenubar();
	g_gui.root->Refresh();

	return true;
}

void EditorManager::OpenMap() {
	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_LOAD_FILE_WILDCARD_OTGZ : MAP_LOAD_FILE_WILDCARD;
	wxFileDialog dialog(g_gui.root, "Open map file", wxEmptyString, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dialog.ShowModal() == wxID_OK) {
		LoadMap(dialog.GetPath());
	}
}

void EditorManager::SaveMap() {
	if (!IsEditorOpen()) {
		return;
	}

	if (GetCurrentMap().hasFile()) {
		SaveCurrentMap(FileName(""), true);
	} else {
		wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
		wxFileDialog dialog(g_gui.root, "Save...", wxEmptyString, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (dialog.ShowModal() == wxID_OK) {
			SaveCurrentMap(dialog.GetPath(), true);
		}
	}
}

void EditorManager::SaveMapAs() {
	if (!IsEditorOpen()) {
		return;
	}

	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
	wxFileDialog dialog(g_gui.root, "Save As...", "", "", wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dialog.ShowModal() == wxID_OK) {
		SaveCurrentMap(dialog.GetPath(), true);
		g_status.UpdateTitle();
		g_gui.root->AddRecentFile(dialog.GetPath());
		g_gui.root->UpdateMenubar();
	}
}

bool EditorManager::LoadMap(const FileName& fileName) {
	spdlog::info("EditorManager::LoadMap - Loading map: {}", nstr(fileName.GetFullPath()));
	g_status.SetStatusText("Loading map...");
	if (g_gui.root) {
		g_gui.root->Update();
	}
	g_gui.FinishWelcomeDialog();

	if (GetCurrentEditor() && !GetCurrentMap().hasChanged() && !GetCurrentMap().hasFile()) {
		CloseCurrentEditor();
	}

	std::unique_ptr<Editor> editor;
	try {
		// Identify version first
		MapVersion ver;
		if (!IOMapOTBM::getVersionInfo(fileName, ver)) {
			throw std::runtime_error(std::format("Could not open file \"{}\".\nThis is not a valid OTBM file or it does not exist.", nstr(fileName.GetFullPath())));
		}

		if (g_version.GetCurrentVersionID() != ver.client) {
			wxString error;
			wxArrayString warnings;
			if (CloseAllEditors()) {
				if (!g_version.LoadVersion(ver.client, error, warnings)) {
					g_status.SetStatusText("Failed to load map.");
					DialogUtil::PopupDialog("Error", error, wxOK);
					return false;
				}
				DialogUtil::ListDialog("Warnings", warnings);
			} else {
				throw std::runtime_error("All maps of different versions were not closed.");
			}
		}

		editor = EditorFactory::LoadFromFile(g_gui.copybuffer, fileName);
	} catch (std::runtime_error& e) {
		g_status.SetStatusText("Failed to load map.");
		DialogUtil::PopupDialog(g_gui.root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	auto* mapTab = newd MapTab(g_gui.tabbook.get(), editor.release());
	mapTab->OnSwitchEditorMode(g_gui.mode);

	g_gui.root->AddRecentFile(fileName);

	mapTab->GetView()->FitToMap();
	g_status.UpdateTitle();
	DialogUtil::ListDialog("Map loader errors", mapTab->GetMap()->getWarnings());
	g_gui.root->DoQueryImportCreatures();

	g_gui.FitViewToMap(mapTab);
	g_gui.root->UpdateMenubar();

	std::string path = g_settings.getString(Config::RECENT_EDITED_MAP_PATH);
	if (!path.empty()) {
		FileName file(path);
		if (file == fileName) {
			std::istringstream stream(g_settings.getString(Config::RECENT_EDITED_MAP_POSITION));
			Position position;
			stream >> position;
			mapTab->SetScreenCenterPosition(position);
		}
	}

	g_status.SetStatusText("Map loaded successfully.");
	return true;
}

void EditorManager::DoCut() {
	if (!g_gui.IsSelectionMode()) {
		return;
	}

	Editor* editor = GetCurrentEditor();
	if (!editor) {
		return;
	}

	editor->copybuffer.cut(*editor, g_gui.GetCurrentFloor());
	g_gui.RefreshView();
	g_gui.root->UpdateMenubar();
}

void EditorManager::DoCopy() {
	if (!g_gui.IsSelectionMode()) {
		return;
	}

	Editor* editor = GetCurrentEditor();
	if (!editor) {
		return;
	}

	editor->copybuffer.copy(*editor, g_gui.GetCurrentFloor());
	g_gui.RefreshView();
	g_gui.root->UpdateMenubar();
}

void EditorManager::DoPaste() {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		g_gui.copybuffer.paste(*mapTab->GetEditor(), mapTab->GetCanvas()->GetCursorPosition());
	}
}

void EditorManager::PreparePaste() {
	Editor* editor = GetCurrentEditor();
	if (editor) {
		g_gui.SetSelectionMode();
		editor->selection.start();
		editor->selection.clear();
		editor->selection.finish();
		StartPasting();
		g_gui.RefreshView();
	}
}

void EditorManager::StartPasting() {
	if (GetCurrentEditor()) {
		g_gui.pasting = true;
		g_gui.secondary_map = &g_gui.copybuffer.getBufferMap();
	}
}

void EditorManager::EndPasting() {
	if (g_gui.pasting) {
		g_gui.pasting = false;
		g_gui.secondary_map = nullptr;
	}
}

bool EditorManager::CanUndo() {
	Editor* editor = GetCurrentEditor();
	return (editor && editor->actionQueue->canUndo());
}

bool EditorManager::CanRedo() {
	Editor* editor = GetCurrentEditor();
	return (editor && editor->actionQueue->canRedo());
}

bool EditorManager::DoUndo() {
	Editor* editor = GetCurrentEditor();
	if (editor && editor->actionQueue->canUndo()) {
		editor->actionQueue->undo();
		if (!editor->selection.empty()) {
			g_gui.SetSelectionMode();
		}
		g_status.SetStatusText("Undo action");
		g_gui.UpdateMinimap();
		g_gui.root->UpdateMenubar();
		g_gui.root->Refresh();
		return true;
	}
	return false;
}

bool EditorManager::DoRedo() {
	Editor* editor = GetCurrentEditor();
	if (editor && editor->actionQueue->canRedo()) {
		editor->actionQueue->redo();
		if (!editor->selection.empty()) {
			g_gui.SetSelectionMode();
		}
		g_status.SetStatusText("Redo action");
		g_gui.UpdateMinimap();
		g_gui.root->UpdateMenubar();
		g_gui.root->Refresh();
		return true;
	}
	return false;
}
