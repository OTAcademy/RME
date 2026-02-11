//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/main_frame.h"

#include "ui/dialog_util.h"
#include "app/application.h"
#include "util/file_system.h"
#include "editor/hotkey_manager.h"

#include "game/sprites.h"
#include "editor/editor.h"
#include "ui/dialogs/goto_position_dialog.h"
#include "palette/palette_window.h"
#include "app/preferences.h"
#include "ui/result_window.h"
#include "rendering/ui/minimap_window.h"
#include "ui/about_window.h"
#include "ui/main_menubar.h"
#include "app/updater.h"
#include "ui/map/export_tilesets_window.h"
#include <wx/stattext.h>
#include <wx/slider.h>

#include "game/materials.h"
#include "map/map.h"
#include "game/complexitem.h"
#include "game/creature.h"

#include <spdlog/spdlog.h>
#include "../brushes/icon/editor_icon.xpm"

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame((wxFrame*)nullptr, -1, title, pos, size, wxDEFAULT_FRAME_STYLE) {
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);

#if wxCHECK_VERSION(3, 1, 0) // 3.1.0 or higher
	// Make sure ShowFullScreen() uses the full screen API on macOS
	EnableFullScreenView(true);
#endif

	// Creates the file-dropdown menu
	menu_bar = std::make_unique<MainMenuBar>(this);
	std::vector<std::string> warnings;
	wxString error;

	wxFileName filename;
	filename.Assign(FileSystem::GetFoundDataDirectory() + "menubar.xml");
	if (!filename.FileExists()) {
		filename = FileName(FileSystem::GetDataDirectory() + "menubar.xml");
	}

	if (!menu_bar->Load(filename, warnings, error)) {
		wxLogError(wxString() + "Could not load menubar.xml, editor will NOT be able to show its menu.\n");
	}

	wxStatusBar* statusbar = CreateStatusBar();
	statusbar->SetFieldsCount(4);
	SetStatusText(wxString("Welcome to ") << __W_RME_APPLICATION_NAME__ << " " << __W_RME_VERSION__);

	// Le sizer
	g_gui.aui_manager = std::make_unique<wxAuiManager>(this);
	g_gui.tabbook = std::make_unique<MapTabbook>(this, wxID_ANY);

	tool_bar = std::make_unique<MainToolBar>(this, g_gui.aui_manager.get());

	g_gui.aui_manager->AddPane(g_gui.tabbook.get(), wxAuiPaneInfo().CenterPane().Floatable(false).CloseButton(false).PaneBorder(false));

	g_gui.aui_manager->Update();

	UpdateMenubar();

	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnExit, this);
#ifdef _USE_UPDATER_
	Bind(EVT_UPDATE_CHECK_FINISHED, &MainFrame::OnUpdateReceived, this);
#endif
	Bind(EVT_UPDATE_MENUS, &MainFrame::OnUpdateMenus, this);
	Bind(wxEVT_IDLE, &MainFrame::OnIdle, this);
}

MainFrame::~MainFrame() = default;

void MainFrame::OnIdle(wxIdleEvent& event) {
	if (menu_bar) {
		menu_bar->Update();
	}
	if (tool_bar) {
		tool_bar->UpdateButtons();
	}
	event.RequestMore();
	event.Skip();
}

#ifdef _USE_UPDATER_
void MainFrame::OnUpdateReceived(wxCommandEvent& event) {
	void* clientData = event.GetClientData();
	if (!clientData) {
		return;
	}
	std::string data = *static_cast<std::string*>(clientData);
	delete static_cast<std::string*>(clientData);

	size_t first_colon = data.find(':');
	size_t second_colon = data.find(':', first_colon + 1);

	if (first_colon == std::string::npos || second_colon == std::string::npos) {
		return;
	}

	std::string update = data.substr(0, first_colon);
	std::string verstr = data.substr(first_colon + 1, second_colon - first_colon - 1);
	std::string url = (second_colon == data.size() ? "" : data.substr(second_colon + 1));

	if (update == "yes") {
		int ret = DialogUtil::PopupDialog(
			"Update Notice",
			wxString("There is a newd update available (") << wxstr(verstr) << "). Do you want to go to the website and download it?",
			wxYES | wxNO,
			"I don't want any update notices",
			Config::AUTOCHECK_FOR_UPDATES
		);
		if (ret == wxID_YES) {
			::wxLaunchDefaultBrowser(wxstr(url), wxBROWSER_NEW_WINDOW);
		}
	}
}
#endif

void MainFrame::OnUpdateMenus(wxCommandEvent&) {
	UpdateMenubar();
	g_gui.UpdateMinimap(true);
	g_gui.UpdateTitle();
}

#ifdef __WINDOWS__
bool MainFrame::MSWTranslateMessage(WXMSG* msg) {
	if (g_hotkeys.AreHotkeysEnabled()) {
		if (wxFrame::MSWTranslateMessage(msg)) {
			return true;
		}
	} else {
		if (wxWindow::MSWTranslateMessage(msg)) {
			return true;
		}
	}
	return false;
}
#endif

void MainFrame::UpdateMenubar() {
	menu_bar->Update();
	tool_bar->UpdateButtons();
}

bool MainFrame::DoQueryClose() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		if (editor->live_manager.IsLive()) {
			long ret = DialogUtil::PopupDialog(
				"Must Close Server",
				wxString("You are currently connected to a live server, to close this map the connection must be severed."),
				wxOK | wxCANCEL
			);
			if (ret == wxID_OK) {
				editor->live_manager.CloseServer();
			} else {
				return false;
			}
		}
	}
	return true;
}

bool MainFrame::DoQuerySaveTileset(bool doclose) {

	if (!g_materials.needSave()) {
		// skip dialog when there is nothing to save
		return true;
	}

	long ret = DialogUtil::PopupDialog(
		"Export tileset",
		"Do you want to export your tileset changes before exiting?",
		wxYES | wxNO | wxCANCEL
	);

	if (ret == wxID_NO) {
		// "no" - exit without saving
		return true;
	} else if (ret == wxID_CANCEL) {
		// "cancel" - just close the dialog
		return false;
	}

	// "yes" button was pressed, open tileset exporting dialog
	if (g_gui.GetCurrentEditor()) {
		ExportTilesetsWindow dlg(this, *g_gui.GetCurrentEditor());
		dlg.ShowModal();
		dlg.Destroy();
	}

	return !g_materials.needSave();
}

bool MainFrame::DoQuerySave(bool doclose) {
	if (!g_gui.IsEditorOpen()) {
		return true;
	}

	if (!DoQuerySaveTileset()) {
		return false;
	}

	Editor& editor = *g_gui.GetCurrentEditor();
	if (editor.live_manager.IsClient()) {
		long ret = DialogUtil::PopupDialog(
			"Disconnect",
			"Do you want to disconnect?",
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor.live_manager.CloseServer();
		return DoQuerySave(doclose);
	} else if (editor.live_manager.IsServer()) {
		long ret = DialogUtil::PopupDialog(
			"Shutdown",
			"Do you want to shut down the server? (any clients will be disconnected)",
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor.live_manager.CloseServer();
		return DoQuerySave(doclose);
	} else if (g_gui.ShouldSave()) {
		long ret = DialogUtil::PopupDialog(
			"Save changes",
			"Do you want to save your changes to \"" + wxstr(g_gui.GetCurrentMap().getName()) + "\"?",
			wxYES | wxNO | wxCANCEL
		);

		if (ret == wxID_YES) {
			if (g_gui.GetCurrentMap().hasFile()) {
				g_gui.SaveCurrentMap(true);
			} else {
				wxFileDialog file(this, "Save...", "", "", "*.otbm", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				int32_t result = file.ShowModal();
				if (result == wxID_OK) {
					g_gui.SaveCurrentMap(file.GetPath(), true);
				} else {
					return false;
				}
			}
		} else if (ret == wxID_CANCEL) {
			return false;
		}
	}

	if (doclose) {
		UnnamedRenderingLock();
		g_gui.CloseCurrentEditor();
	}

	return true;
}

bool MainFrame::DoQueryImportCreatures() {
	if (g_creatures.hasMissing()) {
		long ret = DialogUtil::PopupDialog("Missing creatures", "There are missing creatures and/or NPC in the editor, do you want to load them from an OT monster/npc file?", wxYES | wxNO);
		if (ret == wxID_YES) {
			do {
				wxFileDialog dlg(g_gui.root, "Import monster/npc file", "", "", "*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if (dlg.ShowModal() == wxID_OK) {
					wxArrayString paths;
					dlg.GetPaths(paths);
					for (uint32_t i = 0; i < paths.GetCount(); ++i) {
						wxString error;
						std::vector<std::string> warnings;
						bool ok = g_creatures.importXMLFromOT(FileName(paths[i]), error, warnings);
						if (ok) {
							DialogUtil::ListDialog("Monster loader errors", warnings);
						} else {
							wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_gui.root);
						}
					}
				} else {
					break;
				}
			} while (g_creatures.hasMissing());
		}
	}
	g_gui.RefreshPalettes();
	return true;
}

void MainFrame::UpdateFloorMenu() {
	menu_bar->UpdateFloorMenu();
}

bool MainFrame::LoadMap(FileName name) {
	return g_gui.LoadMap(name);
}

void MainFrame::OnExit(wxCloseEvent& event) {
	// clicking 'x' button

	// do you want to save map changes?
	while (g_gui.IsEditorOpen()) {
		if (!DoQuerySave()) {
			if (event.CanVeto()) {
				event.Veto();
				return;
			} else {
				break;
			}
		}
	}
	g_layout.SavePerspective();

	g_palettes.DestroyPalettes();
	g_minimap.Destroy();
	g_search.HideSearchWindow();

	g_gui.aui_manager->UnInit();
	static_cast<Application&>(wxGetApp()).Unload();
	Destroy();
}

void MainFrame::AddRecentFile(const FileName& file) {
	menu_bar->recentFilesManager.AddFile(file);
}

void MainFrame::LoadRecentFiles() {
	menu_bar->recentFilesManager.Load(&g_settings.getConfigObject());
}

void MainFrame::SaveRecentFiles() {
	menu_bar->recentFilesManager.Save(&g_settings.getConfigObject());
}

std::vector<wxString> MainFrame::GetRecentFiles() {
	return menu_bar->recentFilesManager.GetFiles();
}

void MainFrame::PrepareDC(wxDC& dc) {
	dc.SetLogicalOrigin(0, 0);
	dc.SetAxisOrientation(1, 0);
	dc.SetUserScale(1.0, 1.0);
	dc.SetMapMode(wxMM_TEXT);
}
