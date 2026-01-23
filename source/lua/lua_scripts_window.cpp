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

#include "main.h"
#include "lua_scripts_window.h"
#include "lua_script_manager.h"
#include "../gui_ids.h"

#include <wx/filename.h>
#include <wx/stdpaths.h>

// Static instance
LuaScriptsWindow* LuaScriptsWindow::instance = nullptr;

BEGIN_EVENT_TABLE(LuaScriptsWindow, wxPanel)
	EVT_LIST_ITEM_ACTIVATED(SCRIPT_MANAGER_LIST, LuaScriptsWindow::OnScriptActivated)
	EVT_LIST_ITEM_SELECTED(SCRIPT_MANAGER_LIST, LuaScriptsWindow::OnScriptSelected)
	EVT_BUTTON(SCRIPT_MANAGER_RELOAD, LuaScriptsWindow::OnReloadScripts)
	EVT_BUTTON(SCRIPT_MANAGER_OPEN_FOLDER, LuaScriptsWindow::OnOpenFolder)
	EVT_BUTTON(SCRIPT_MANAGER_CLEAR_CONSOLE, LuaScriptsWindow::OnClearConsole)
	EVT_BUTTON(SCRIPT_MANAGER_RUN_SCRIPT, LuaScriptsWindow::OnRunScript)
END_EVENT_TABLE()

LuaScriptsWindow::LuaScriptsWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	script_list(nullptr),
	console_output(nullptr),
	reload_button(nullptr),
	open_folder_button(nullptr),
	clear_console_button(nullptr),
	run_script_button(nullptr) {
	BuildUI();
	RefreshScriptList();

	// Set up output callback
	g_luaScripts.setOutputCallback([this](const std::string& msg, bool isError) {
		// Must be called from main thread
		if (wxThread::IsMain()) {
			LogMessage(wxString::FromUTF8(msg), isError);
		} else {
			// Post event to main thread
			wxTheApp->CallAfter([this, msg, isError]() {
				LogMessage(wxString::FromUTF8(msg), isError);
			});
		}
	});
}

LuaScriptsWindow::~LuaScriptsWindow() {
	// Clear the callback
	g_luaScripts.setOutputCallback(nullptr);

	if (instance == this) {
		instance = nullptr;
	}
}

void LuaScriptsWindow::BuildUI() {
	wxBoxSizer* mainSizer = newd wxBoxSizer(wxVERTICAL);

	// Button bar
	wxBoxSizer* buttonSizer = newd wxBoxSizer(wxHORIZONTAL);

	reload_button = newd wxButton(this, SCRIPT_MANAGER_RELOAD, "Reload All");
	reload_button->SetToolTip("Reload all scripts from disk");
	buttonSizer->Add(reload_button, 0, wxALL, 2);

	open_folder_button = newd wxButton(this, SCRIPT_MANAGER_OPEN_FOLDER, "Open Folder");
	open_folder_button->SetToolTip("Open scripts folder in file explorer");
	buttonSizer->Add(open_folder_button, 0, wxALL, 2);

	run_script_button = newd wxButton(this, SCRIPT_MANAGER_RUN_SCRIPT, "Run");
	run_script_button->SetToolTip("Run selected script");
	run_script_button->Enable(false);
	buttonSizer->Add(run_script_button, 0, wxALL, 2);

	buttonSizer->AddStretchSpacer();

	clear_console_button = newd wxButton(this, SCRIPT_MANAGER_CLEAR_CONSOLE, "Clear");
	clear_console_button->SetToolTip("Clear console output");
	buttonSizer->Add(clear_console_button, 0, wxALL, 2);

	mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 2);

	// Script list
	script_list = newd wxListCtrl(this, SCRIPT_MANAGER_LIST,
		wxDefaultPosition, wxSize(-1, 150),
		wxLC_REPORT | wxLC_SINGLE_SEL);

	script_list->InsertColumn(0, "Status", wxLIST_FORMAT_CENTER, 40);
	script_list->InsertColumn(1, "Title", wxLIST_FORMAT_LEFT, 70);
	script_list->InsertColumn(2, "Description", wxLIST_FORMAT_LEFT, 150);
	script_list->InsertColumn(3, "Author", wxLIST_FORMAT_LEFT, 70);
	script_list->InsertColumn(4, "Version", wxLIST_FORMAT_LEFT, 40);
	script_list->InsertColumn(5, "Shortcut", wxLIST_FORMAT_LEFT, 50);

	mainSizer->Add(script_list, 1, wxEXPAND | wxALL, 2);

	// Console label
	wxStaticText* consoleLabel = newd wxStaticText(this, wxID_ANY, "Console Output:");
	mainSizer->Add(consoleLabel, 0, wxLEFT | wxTOP, 4);

	// Console output
	console_output = newd wxTextCtrl(this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxSize(-1, 100),
		wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxHSCROLL);

	// Set monospace font for console
	wxFont consoleFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	console_output->SetFont(consoleFont);
	console_output->SetBackgroundColour(wxColour(30, 30, 30));
	console_output->SetForegroundColour(wxColour(200, 200, 200));

	mainSizer->Add(console_output, 1, wxEXPAND | wxALL, 2);

	SetSizer(mainSizer);
}

void LuaScriptsWindow::RefreshScriptList() {
	if (!script_list) return;

	script_list->DeleteAllItems();

	const auto& scripts = g_luaScripts.getScripts();
	for (size_t i = 0; i < scripts.size(); ++i) {
		const auto& script = scripts[i];

		long index = script_list->InsertItem(i, script->isEnabled() ? "On" : "Off");


		script_list->SetItem(index, 1, wxString::FromUTF8(script->getDisplayName()));
		script_list->SetItem(index, 2, wxString::FromUTF8(script->getDescription()));
		script_list->SetItem(index, 3, wxString::FromUTF8(script->getAuthor()));
		script_list->SetItem(index, 4, wxString::FromUTF8(script->getVersion()));
		script_list->SetItem(index, 5, wxString::FromUTF8(script->getShortcut()));

		// Store script index as item data
		script_list->SetItemData(index, static_cast<long>(i));

		// Color based on enabled state
		if (!script->isEnabled()) {
			script_list->SetItemTextColour(index, wxColour(128, 128, 128));
		}
	}

	// Auto-resize columns if needed
	if (scripts.size() > 0) {
		script_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
		// Description (index 2) keeps fixed width
		script_list->SetColumnWidth(3, wxLIST_AUTOSIZE);
		script_list->SetColumnWidth(4, wxLIST_AUTOSIZE);
		script_list->SetColumnWidth(5, wxLIST_AUTOSIZE);
	}
}

void LuaScriptsWindow::LogMessage(const wxString& message, bool isError) {
	if (!console_output) return;

	// Set color based on message type
	wxTextAttr attr;
	if (isError) {
		attr.SetTextColour(wxColour(255, 100, 100));  // Red for errors
	} else {
		attr.SetTextColour(wxColour(200, 200, 200));  // Light gray for normal
	}

	console_output->SetDefaultStyle(attr);

	// Add timestamp
	wxDateTime now = wxDateTime::Now();
	wxString timestamp = now.Format("[%H:%M:%S] ");

	console_output->AppendText(timestamp + message);
	if (!message.EndsWith("\n")) {
		console_output->AppendText("\n");
	}

	// Scroll to end
	console_output->ShowPosition(console_output->GetLastPosition());
}

void LuaScriptsWindow::ClearConsole() {
	if (console_output) {
		console_output->Clear();
	}
}

void LuaScriptsWindow::UpdateScriptState(long index) {
	if (!script_list || index < 0) return;

	size_t scriptIndex = static_cast<size_t>(script_list->GetItemData(index));
	const auto& scripts = g_luaScripts.getScripts();

	if (scriptIndex < scripts.size()) {
		const auto& script = scripts[scriptIndex];
		script_list->SetItem(index, 0, script->isEnabled() ? "On" : "Off");

		if (script->isEnabled()) {
			script_list->SetItemTextColour(index, wxColour(0, 0, 0));
		} else {
			script_list->SetItemTextColour(index, wxColour(128, 128, 128));
		}
	}
}

void LuaScriptsWindow::OnScriptActivated(wxListEvent& event) {
	// Double-click to run the script
	long index = event.GetIndex();
	if (index < 0) return;

	size_t scriptIndex = static_cast<size_t>(script_list->GetItemData(index));
	const auto& scripts = g_luaScripts.getScripts();

	if (scriptIndex < scripts.size()) {
		const auto& script = scripts[scriptIndex];
		if (script->isEnabled()) {
			LogMessage("Running: " + wxString::FromUTF8(script->getDisplayName()));
			std::string error;
			if (!g_luaScripts.executeScript(scriptIndex, error)) {
				LogMessage("Error: " + wxString::FromUTF8(error), true);
			}
		} else {
			LogMessage("Script is disabled: " + wxString::FromUTF8(script->getDisplayName()), true);
		}
	}
}

void LuaScriptsWindow::OnScriptSelected(wxListEvent& event) {
	// Enable/disable run button based on selection
	run_script_button->Enable(event.GetIndex() >= 0);
}

void LuaScriptsWindow::OnReloadScripts(wxCommandEvent& event) {
	LogMessage("Reloading scripts...");
	g_luaScripts.reloadScripts();
	RefreshScriptList();
	LogMessage("Scripts reloaded. Found " + wxString::Format("%zu", g_luaScripts.getScripts().size()) + " scripts.");
}

void LuaScriptsWindow::OnOpenFolder(wxCommandEvent& event) {
	wxString scriptsPath = g_luaScripts.getScriptsDirectory();

	// Ensure directory exists
	if (!wxDirExists(scriptsPath)) {
		wxMkdir(scriptsPath);
	}

#ifdef _WIN32
	wxExecute("explorer \"" + scriptsPath + "\"", wxEXEC_ASYNC);
#elif defined(__APPLE__)
	wxExecute("open \"" + scriptsPath + "\"", wxEXEC_ASYNC);
#else
	wxExecute("xdg-open \"" + scriptsPath + "\"", wxEXEC_ASYNC);
#endif
}

void LuaScriptsWindow::OnClearConsole(wxCommandEvent& event) {
	ClearConsole();
}

void LuaScriptsWindow::OnRunScript(wxCommandEvent& event) {
	long selected = script_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selected < 0) return;

	size_t scriptIndex = static_cast<size_t>(script_list->GetItemData(selected));
	const auto& scripts = g_luaScripts.getScripts();

	if (scriptIndex < scripts.size()) {
		const auto& script = scripts[scriptIndex];
		LogMessage("Running: " + wxString::FromUTF8(script->getDisplayName()));
		std::string error;
		if (!g_luaScripts.executeScript(scriptIndex, error)) {
			LogMessage("Error: " + wxString::FromUTF8(error), true);
		}
	}
}

void LuaScriptsWindow::OnScriptCheckToggle(wxListEvent& event) {
	// Toggle script enabled state (would need checkbox implementation)
	long index = event.GetIndex();
	if (index < 0) return;

	size_t scriptIndex = static_cast<size_t>(script_list->GetItemData(index));
	g_luaScripts.setScriptEnabled(scriptIndex, !g_luaScripts.isScriptEnabled(scriptIndex));
	UpdateScriptState(index);
}
