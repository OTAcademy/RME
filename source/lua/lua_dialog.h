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

#ifndef RME_LUA_DIALOG_H
#define RME_LUA_DIALOG_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/slider.h>
#include <wx/clrpicker.h>
#include <wx/filepicker.h>
#include <wx/statbmp.h>

#include "lua_api_image.h"

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <stack>

// Forward declaration
class LuaDialog;

// Widget info stored for each widget added to the dialog
struct LuaDialogWidget {
	std::string id;
	std::string type;
	wxWindow* widget = nullptr;
	sol::function onchange;
	sol::function onclick;
	sol::function ondoubleclick;
	sol::function oncontextmenu;
	sol::function onleftclick;
	sol::function onrightclick;
};

struct LuaDialogTab {
	std::string id;
	std::string text;
	bool isButton = false;
	sol::function onclick;
	sol::function oncontextmenu;
};

// Lua Dialog wrapper class
// Provides Aseprite-style method chaining API for creating dialogs
class LuaDialog : public wxDialog {
public:
	LuaDialog(const std::string& title, sol::this_state ts);
	LuaDialog(sol::table options, sol::this_state ts);
	virtual ~LuaDialog();

	// Method chaining widget methods - all return pointer to self for chaining
	LuaDialog* label(sol::table options);
	LuaDialog* input(sol::table options);
	LuaDialog* number(sol::table options);
	LuaDialog* slider(sol::table options);
	LuaDialog* check(sol::table options);
	LuaDialog* radio(sol::table options);
	LuaDialog* combobox(sol::table options);
	LuaDialog* button(sol::table options);
	LuaDialog* color(sol::table options);
	LuaDialog* item(sol::table options);
	LuaDialog* mapCanvas(sol::table options);
	LuaDialog* list(sol::table options);
	LuaDialog* grid(sol::table options);
	LuaDialog* file(sol::table options);
	LuaDialog* image(sol::table options);
	LuaDialog* separator(sol::optional<sol::table> options);
	LuaDialog* newrow();
	LuaDialog* tab(sol::table options);

	LuaDialog* endtabs();

	// Nesting
	LuaDialog* wrap(sol::table options); // Define explicit horizontal wrapper
	LuaDialog* endwrap();

	// Layout containers
	LuaDialog* box(sol::table options); // Generic box sizer
	LuaDialog* endbox();

	LuaDialog* panel(sol::table options);
	LuaDialog* endpanel();

	// Dialog control
	LuaDialog* show(sol::optional<sol::table> options);
	void close();
	LuaDialog* modify(sol::table options);
	void repaint();
	void clear();
	void layout();

	// Data access
	sol::table getData();
	void setData(sol::table data);

	// Bounds access
	sol::table getBounds();
	void setBounds(sol::table bounds);
	sol::object getActiveTab();

	bool isDockable() const {
		return dockPanel != nullptr;
	}

private:
	sol::state_view lua;
	std::vector<LuaDialogWidget> widgets;
	std::map<std::string, sol::object> values;
	sol::function oncloseCallback;

	std::stack<wxSizer*> sizerStack;
	std::stack<wxWindow*> panelStack;
	std::stack<wxBoxSizer*> rowSizerStack;
	std::vector<wxSizer*> tabSizers;
	// Parent window for widget creation (usually dialog, but can be other panels)
	// Actually, keeping text/buttons parenting to dialog is easier for events.
	// As long as sizer hierarchy is correct, parent hierarchy is less strict in wx, but static box sizer needs static box sibling.
	// Let's rely on 'getParentForWidget()' which currently uses 'currentTabPanel'.

	wxBoxSizer* mainSizer = nullptr;
	wxBoxSizer* currentRowSizer = nullptr;
	wxNotebook* currentNotebook = nullptr;
	wxNotebook* activeNotebook = nullptr;
	wxPanel* currentTabPanel = nullptr;
	wxBoxSizer* currentTabSizer = nullptr;
	int hotkeySuspendCount = 0;
	bool hotkeysDisabledByDialog = false;
	bool notebookEventsBound = false;
	bool suppressTabButtonClick = false;
	int suppressTabButtonIndex = -1;

	bool isShowing = false;
	bool waitMode = true;
	std::vector<LuaDialogTab> tabInfos;

	void createLayout();
	void ensureRowSizer();
	void finishCurrentRow();
	wxWindow* getParentForWidget();
	wxSizer* getSizerForWidget();

	void updateValue(const std::string& id);
	void collectAllValues();
	void applyCommonOptions(wxWindow* widget, sol::table options);
	int getSizerFlags(sol::table options, int defaultFlags = 0);
	int getSizerBorder(sol::table options);
	void suspendHotkeys();
	void resumeHotkeys();
	sol::table makeTabInfoTable(int index);
	void handleTabButtonClick(int index);
	void handleTabContextMenu(int index, const wxPoint& screenPos);
	void popupContextMenu(const sol::function& callback, sol::table info, wxWindow* window, const wxPoint& screenPos);

	void onWidgetChange(const std::string& id);
	void onButtonClick(const std::string& id);
	void onWidgetDoubleClick(const std::string& id);

	// Event handlers
	void OnClose(wxCloseEvent& event);

	DECLARE_EVENT_TABLE()

	int reqWidth = -1;
	int reqHeight = -1;
	int reqX = -1;
	int reqY = -1;

public:
	wxPanel* dockPanel = nullptr;
};

namespace LuaAPI {
	// Register the Dialog class with Lua
	void registerDialog(sol::state& lua);
}

#endif // RME_LUA_DIALOG_H
