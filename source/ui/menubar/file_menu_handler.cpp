#include "ui/menubar/file_menu_handler.h"
#include "app/application.h"
#include "app/main.h"
#include "ui/gui.h"
#include "ui/map/export_tilesets_window.h"
#include "ui/map/export_minimap_window.h"
#include "ui/map/import_map_window.h"
#include "ui/dialog_util.h"
#include "ui/about_window.h"
#include "ui/dat_debug_view.h"
#include "app/preferences.h"
#include "ui/extension_window.h"
#include "game/creatures.h"
#include "app/managers/version_manager.h"
#include "ui/controls/sortable_list_box.h"

FileMenuHandler::FileMenuHandler(MainFrame* frame, MainMenuBar* menubar) :
	frame(frame), menubar(menubar) {
}

FileMenuHandler::~FileMenuHandler() {
}

void FileMenuHandler::OnNew(wxCommandEvent& WXUNUSED(event)) {
	g_gui.NewMap();
}

void FileMenuHandler::OnOpen(wxCommandEvent& WXUNUSED(event)) {
	g_gui.OpenMap();
}

void FileMenuHandler::OnSave(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SaveMap();
}

void FileMenuHandler::OnSaveAs(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SaveMapAs();
}

void FileMenuHandler::OnClose(wxCommandEvent& WXUNUSED(event)) {
	frame->DoQuerySave(true); // It closes the editor too
}

void FileMenuHandler::OnQuit(wxCommandEvent& WXUNUSED(event)) {
	g_gui.root->Close();
}

void FileMenuHandler::OnGenerateMap(wxCommandEvent& WXUNUSED(event)) {
	// Not implemented in original code
}

void FileMenuHandler::OnImportMap(wxCommandEvent& WXUNUSED(event)) {
	ASSERT(g_gui.GetCurrentEditor());
	wxDialog* importmap = newd ImportMapWindow(frame, *g_gui.GetCurrentEditor());
	importmap->ShowModal();
}

void FileMenuHandler::OnImportMonsterData(wxCommandEvent& WXUNUSED(event)) {
	wxFileDialog dlg(g_gui.root, "Import monster/npc file", "", "", "*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) {
		wxArrayString paths;
		dlg.GetPaths(paths);
		for (uint32_t i = 0; i < paths.GetCount(); ++i) {
			wxString error;
			wxArrayString warnings;
			bool ok = g_creatures.importXMLFromOT(FileName(paths[i]), error, warnings);
			if (ok) {
				DialogUtil::ListDialog("Monster loader errors", warnings);
			} else {
				wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_gui.root);
			}
		}
	}
}

void FileMenuHandler::OnImportMinimap(wxCommandEvent& WXUNUSED(event)) {
	ASSERT(g_gui.IsEditorOpen());
}

void FileMenuHandler::OnExportMinimap(wxCommandEvent& WXUNUSED(event)) {
	if (g_gui.GetCurrentEditor()) {
		ExportMiniMapWindow dlg(frame, *g_gui.GetCurrentEditor());
		dlg.ShowModal();
		dlg.Destroy();
	}
}

void FileMenuHandler::OnExportTilesets(wxCommandEvent& WXUNUSED(event)) {
	if (g_gui.GetCurrentEditor()) {
		ExportTilesetsWindow dlg(frame, *g_gui.GetCurrentEditor());
		dlg.ShowModal();
		dlg.Destroy();
	}
}

void FileMenuHandler::OnReloadDataFiles(wxCommandEvent& WXUNUSED(event)) {
	wxString error;
	wxArrayString warnings;
	g_version.LoadVersion(g_version.GetCurrentVersionID(), error, warnings, true);
	DialogUtil::PopupDialog("Error", error, wxOK);
	DialogUtil::ListDialog("Warnings", warnings);
}

void FileMenuHandler::OnPreferences(wxCommandEvent& WXUNUSED(event)) {
	PreferencesWindow dialog(frame);
	dialog.ShowModal();
	dialog.Destroy();
}

void FileMenuHandler::OnListExtensions(wxCommandEvent& WXUNUSED(event)) {
	ExtensionsDialog exts(frame);
	exts.ShowModal();
}

void FileMenuHandler::OnGotoWebsite(wxCommandEvent& WXUNUSED(event)) {
	::wxLaunchDefaultBrowser(__SITE_URL__, wxBROWSER_NEW_WINDOW);
}

void FileMenuHandler::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	AboutWindow about(frame);
	about.ShowModal();
}
