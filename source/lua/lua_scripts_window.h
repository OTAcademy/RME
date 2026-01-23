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

#ifndef RME_LUA_SCRIPTS_WINDOW_H
#define RME_LUA_SCRIPTS_WINDOW_H

#include <wx/listctrl.h>
#include <wx/textctrl.h>

class LuaScriptsWindow : public wxPanel {
public:
	LuaScriptsWindow(wxWindow* parent);
	virtual ~LuaScriptsWindow();

	// Refresh the script list from LuaScriptManager
	void RefreshScriptList();

	// Log a message to the console
	void LogMessage(const wxString& message, bool isError = false);

	// Clear the console
	void ClearConsole();

	// Get singleton instance (created by application)
	static LuaScriptsWindow* Get() {
		return instance;
	}
	static void SetInstance(LuaScriptsWindow* win) {
		instance = win;
	}

protected:
	// Event handlers
	void OnScriptActivated(wxListEvent& event);
	void OnScriptSelected(wxListEvent& event);
	void OnReloadScripts(wxCommandEvent& event);
	void OnOpenFolder(wxCommandEvent& event);
	void OnClearConsole(wxCommandEvent& event);
	void OnRunScript(wxCommandEvent& event);
	void OnScriptCheckToggle(wxListEvent& event);

	// Build the UI
	void BuildUI();

	// Update script enable state in list
	void UpdateScriptState(long index);

private:
	wxListCtrl* script_list;
	wxTextCtrl* console_output;
	wxButton* reload_button;
	wxButton* open_folder_button;
	wxButton* clear_console_button;
	wxButton* run_script_button;

	static LuaScriptsWindow* instance;

	DECLARE_EVENT_TABLE()
};

#endif // RME_LUA_SCRIPTS_WINDOW_H
