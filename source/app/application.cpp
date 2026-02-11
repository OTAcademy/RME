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

#include <wx/snglinst.h>
#include <spdlog/spdlog.h>

#include "../brushes/icon/editor_icon.xpm"

wxIMPLEMENT_APP(Application);

Application::~Application() {
	// Destroy
}

bool Application::OnInit() {
	// Enable modern appearance handling (wxWidgets 3.3+)
#if wxCHECK_VERSION(3, 3, 0)
	SetAppearance(wxApp::Appearance::System);
#endif

#ifdef __WXMSW__
	// Enable dark mode support for Windows
	MSWEnableDarkMode(wxApp::DarkMode_Always);
#endif

#if defined __DEBUG_MODE__ && defined __WINDOWS__
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef __WINDOWS__
	// Allocate console for debug output
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

	// Configure spdlog for debug output
	spdlog::set_level(spdlog::level::debug);
	spdlog::flush_on(spdlog::level::info);
	spdlog::info("RME starting up - logging enabled");

	spdlog::info("This is free software: you are free to change and redistribute it.");
	spdlog::info("There is NO WARRANTY, to the extent permitted by law.");
	spdlog::info("Review COPYING in RME distribution for details.");

	// Discover data directory
	FileSystem::DiscoverDataDirectory("clients.xml");

	// Tell that we are the real thing
	wxAppConsole::SetInstance(this);

#if defined(__LINUX__) || defined(__WINDOWS__)
	int argc = 1;
	char* argv[1] = { wxString(this->argv[0]).char_str() };
	// glutInit(&argc, argv);
#endif

	// Load some internal stuff
	g_settings.load();
	FixVersionDiscrapencies();
	g_hotkeys.LoadHotkeys();
	ClientVersion::loadVersions();

#ifdef _USE_PROCESS_COM
	m_single_instance_checker = newd wxSingleInstanceChecker; // Instance checker has to stay alive throughout the applications lifetime
	if (g_settings.getInteger(Config::ONLY_ONE_INSTANCE) && m_single_instance_checker->IsAnotherRunning()) {
		RMEProcessClient client;
		wxConnectionBase* connection = client.MakeConnection("localhost", "rme_host", "rme_talk");
		if (connection) {
			wxString fileName;
			if (ParseCommandLineMap(fileName)) {
				wxLogNull nolog; // We might get a timeout message if the file fails to open on the running instance. Let's not show that message.
				connection->Execute(fileName);
			}
			connection->Disconnect();
			wxDELETE(connection);
		}
		wxDELETE(m_single_instance_checker);
		return false; // Since we return false - OnExit is never called
	}
	// We act as server then
	m_proc_server = newd RMEProcessServer();
	if (!m_proc_server->Create("rme_host")) {
		wxLogWarning("Could not register IPC service!");
	}
#endif

	// Image handlers
	wxInitAllImageHandlers();

	g_gui.gfx.loadEditorSprites();

#ifndef __DEBUG_MODE__
	// wxHandleFatalExceptions(true);
#endif
	// Load all the dependency files
	std::string error;
	StringVector warnings;

	m_file_to_open = wxEmptyString;
	ParseCommandLineMap(m_file_to_open);

	g_gui.root = newd MainFrame(__W_RME_APPLICATION_NAME__, wxDefaultPosition, wxSize(700, 500));
	SetTopWindow(g_gui.root);
	g_gui.SetTitle("");

	g_gui.root->LoadRecentFiles();

	// Load palette
	g_gui.LoadPerspective();

	wxIcon icon(editor_icon);
	g_gui.root->SetIcon(icon);

	if (g_settings.getInteger(Config::WELCOME_DIALOG) == 1 && m_file_to_open == wxEmptyString) {
		g_gui.ShowWelcomeDialog(icon);
	} else {
		g_gui.root->Show();
	}

	// Set idle event handling mode
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	// Goto RME website?
	if (g_settings.getInteger(Config::GOTO_WEBSITE_ON_BOOT) == 1) {
		::wxLaunchDefaultBrowser(__SITE_URL__, wxBROWSER_NEW_WINDOW);
		g_settings.setInteger(Config::GOTO_WEBSITE_ON_BOOT, 0);
	}

	// Check for updates
#ifdef _USE_UPDATER_
	if (g_settings.getInteger(Config::USE_UPDATER) == -1) {
		int ret = DialogUtil::PopupDialog(
			"Notice",
			"Do you want the editor to automatically check for updates?\n"
			"It will connect to the internet if you choose yes.\n"
			"You can change this setting in the preferences later.",
			wxYES | wxNO
		);
		if (ret == wxID_YES) {
			g_settings.setInteger(Config::USE_UPDATER, 1);
		} else {
			g_settings.setInteger(Config::USE_UPDATER, 0);
		}
	}
	if (g_settings.getInteger(Config::USE_UPDATER) == 1) {
		// UpdateChecker updater;
		// updater.connect(g_gui.root);
	}
#endif

	FileName save_failed_file = FileSystem::GetLocalDataDirectory();
	save_failed_file.SetName(".saving.txt");
	if (save_failed_file.FileExists()) {
		std::ifstream f(nstr(save_failed_file.GetFullPath()).c_str(), std::ios::in);

		std::string backup_otbm, backup_house, backup_spawn;

		getline(f, backup_otbm);
		getline(f, backup_house);
		getline(f, backup_spawn);

		// Remove the file
		f.close();
		std::remove(nstr(save_failed_file.GetFullPath()).c_str());

		// Query file retrieval if possible
		if (!backup_otbm.empty()) {
			long ret = DialogUtil::PopupDialog(
				"Editor Crashed",
				wxString(
					"IMPORTANT! THE EDITOR CRASHED WHILE SAVING!\n\n"
					"Do you want to recover the lost map? (it will be opened immediately):\n"
				) << wxstr(backup_otbm)
				  << "\n"
				  << wxstr(backup_house) << "\n"
				  << wxstr(backup_spawn) << "\n",
				wxYES | wxNO
			);

			if (ret == wxID_YES) {
				// Recover if the user so wishes
				std::remove(backup_otbm.substr(0, backup_otbm.size() - 1).c_str());
				std::rename(backup_otbm.c_str(), backup_otbm.substr(0, backup_otbm.size() - 1).c_str());

				if (!backup_house.empty()) {
					std::remove(backup_house.substr(0, backup_house.size() - 1).c_str());
					std::rename(backup_house.c_str(), backup_house.substr(0, backup_house.size() - 1).c_str());
				}
				if (!backup_spawn.empty()) {
					std::remove(backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
					std::rename(backup_spawn.c_str(), backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
				}

				// Load the map
				g_gui.LoadMap(wxstr(backup_otbm.substr(0, backup_otbm.size() - 1)));
				return true;
			}
		}
	}
	// Keep track of first event loop entry
	m_startup = true;
	return true;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop) {

	// First startup?
	if (!m_startup) {
		return;
	}
	m_startup = false;

	// Don't try to create a map if we didn't load the client map.
	if (ClientVersion::getLatestVersion() == nullptr) {
		return;
	}

	// Open a map.
	if (m_file_to_open != wxEmptyString) {
		g_gui.LoadMap(FileName(m_file_to_open));
	} else if (!g_gui.IsWelcomeDialogShown() && g_gui.NewMap()) { // Open a new empty map
		// You generally don't want to save this map...
		g_gui.GetCurrentEditor()->map.clearChanges();
	}
}

void Application::MacOpenFiles(const wxArrayString& fileNames) {
	if (!fileNames.IsEmpty()) {
		g_gui.LoadMap(FileName(fileNames.Item(0)));
	}
}

void Application::FixVersionDiscrapencies() {
	// Here the registry should be fixed, if the version has been changed
	if (g_settings.getInteger(Config::VERSION_ID) < MAKE_VERSION_ID(1, 0, 5)) {
		g_settings.setInteger(Config::USE_MEMCACHED_SPRITES_TO_SAVE, 0);
	}

	if (g_settings.getInteger(Config::VERSION_ID) < __RME_VERSION_ID__ && ClientVersion::getLatestVersion() != nullptr) {
		g_settings.setInteger(Config::DEFAULT_CLIENT_VERSION, ClientVersion::getLatestVersion()->getID());
	}

	wxString ss = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	if (ss.empty()) {
		ss = wxStandardPaths::Get().GetDocumentsDir();
#ifdef __WINDOWS__
		ss += "/My Pictures/RME/";
#endif
	}
	g_settings.setString(Config::SCREENSHOT_DIRECTORY, nstr(ss));

	// Set registry to newest version
	g_settings.setInteger(Config::VERSION_ID, __RME_VERSION_ID__);
}

void Application::Unload() {
	g_gui.CloseAllEditors();
	g_version.UnloadVersion();
	g_hotkeys.SaveHotkeys();
	g_gui.root->SaveRecentFiles();
	ClientVersion::saveVersions();
	ClientVersion::unloadVersions();
	g_settings.save(true);
	g_gui.root = nullptr;
}

int Application::OnExit() {
#ifdef _USE_PROCESS_COM
	wxDELETE(m_proc_server);
	wxDELETE(m_single_instance_checker);
#endif
	return 1;
}

void Application::OnFatalException() {
	////
}

bool Application::ParseCommandLineMap(wxString& fileName) {
	if (argc == 2) {
		fileName = wxString(argv[1]);
		return true;
	} else if (argc == 3) {
		if (argv[1] == "-ws") {
			g_settings.setInteger(Config::WELCOME_DIALOG, argv[2] == "1" ? 1 : 0);
		}
	}
	return false;
}
