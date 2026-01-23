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
#include "lua_dialog.h"
#include "lua_api_image.h"
#include "../gui.h"
#include "../items.h"
#include "../map_display.h"
#include "../map_drawer.h"
#include "../editor.h"
#include <wx/msgdlg.h>
#include "../common_windows.h"
#include "../find_item_window.h"
#include "../dcbutton.h"
#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/aui/aui.h>
#include <wx/statbmp.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/menu.h>
#include <wx/utils.h>
#ifdef __WXMSW__
	#include <commctrl.h>
#endif

using namespace std::string_literals;

// Specialized Canvas for Lua Dialogs
// Specialized Canvas for Lua Dialogs
class MapPreviewCanvas : public MapCanvas {
public:
	MapPreviewCanvas(wxWindow* parent, Editor& editor) :
		MapCanvas(parent, editor, nullptr),
		view_x(0),
		view_y(0) {

		floor = 7;
		zoom = 1.0;

		// Force Ingame mode for "Client Box" behavior
		drawer->getOptions().SetIngame();
		drawer->getOptions().show_ingame_box = true;
	}

	virtual ~MapPreviewCanvas() {
	}

	// Overrides to decouple from MapWindow
	void SetZoom(double value) override {
		if (value < 0.125) {
			value = 0.125;
		}
		if (value > 25.00) {
			value = 25.0;
		}
		zoom = value;
		wxGLCanvas::Refresh();
	}

	void GetViewBox(int* view_scroll_x, int* view_scroll_y, int* screensize_x, int* screensize_y) const override {
		wxSize size = GetClientSize();
		*screensize_x = size.GetWidth();
		*screensize_y = size.GetHeight();
		*view_scroll_x = view_x;
		*view_scroll_y = view_y;
	}

	void ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y) override {
		screen_x *= GetContentScaleFactor();
		screen_y *= GetContentScaleFactor();

		if (screen_x < 0) {
			*map_x = (view_x + screen_x) / 32;
		} else {
			*map_x = int(view_x + (screen_x * zoom)) / 32;
		}

		if (screen_y < 0) {
			*map_y = (view_y + screen_y) / 32;
		} else {
			*map_y = int(view_y + (screen_y * zoom)) / 32;
		}

		if (floor <= 7) {
			*map_x += 7 - floor;
			*map_y += 7 - floor;
		}
	}

	void GetScreenCenter(int* map_x, int* map_y) override {
		wxSize size = GetClientSize();
		ScreenToMap(size.GetWidth() / 2, size.GetHeight() / 2, map_x, map_y);
	}

	void SetPosition(int x, int y, int z) {
		wxSize size = GetClientSize();
		int width = size.GetWidth();
		int height = size.GetHeight();

		// If not yet laid out, use a default size to avoid bad coordinates
		if (width <= 0) {
			width = 400;
		}
		if (height <= 0) {
			height = 300;
		}

		view_x = (x * 32) - (width * zoom) / 2;
		view_y = (y * 32) - (height * zoom) / 2;
		floor = z;
		Refresh();
	}

	// Disable status bar updates for preview canvases
	void UpdatePositionStatus(int x = -1, int y = -1) { }
	void UpdateZoomStatus() { }
	void Refresh() {
		wxGLCanvas::Refresh();
	}

	void OnMouseMove(wxMouseEvent& event) {
		cursor_x = event.GetX();
		cursor_y = event.GetY();
	}

	void OnWheel(wxMouseEvent& event) {
		// Do nothing - disabled to prevent confusion and potential issues
	}

	void OnGainMouse(wxMouseEvent& event) {
		Refresh();
	}
	void OnLoseMouse(wxMouseEvent& event) {
		Refresh();
	}
	void OnMouseLeftClick(wxMouseEvent& event) {
		SetFocus();
	}
	void OnMouseLeftRelease(wxMouseEvent& event) { }
	void OnMouseRightClick(wxMouseEvent& event) { }
	void OnMouseRightRelease(wxMouseEvent& event) { }
	void OnMouseCenterClick(wxMouseEvent& event) { }
	void OnMouseCenterRelease(wxMouseEvent& event) { }

	void SetViewSize(int w, int h) {
		client_w = w;
		client_h = h;

		// Auto-resize the window to fit the new client dimensions
		int tile_pixel_size = static_cast<int>(32 / zoom);
		int req_w = w * tile_pixel_size;
		int req_h = h * tile_pixel_size;

		// Add some padding for the frame borders if needed, but SetClientSize should handle content area
		// We set the MinSize of the canvas to enforce this size
		SetMinSize(wxSize(req_w, req_h));

		// Attempt to resize the parent dialog to fit the new canvas size
		wxWindow* parent = GetParent();
		if (parent) {
			parent->Fit();
		}

		Refresh();
	}

	void SetLight(bool on) {
		drawer->getOptions().show_lights = on;
		Refresh();
	}

	void SyncView() {
		Editor* current_editor = g_gui.GetCurrentEditor();
		if (!current_editor) {
			return;
		}

		MapTab* tab = g_gui.GetCurrentMapTab();
		if (tab) {
			int cx, cy;
			tab->GetCanvas()->GetScreenCenter(&cx, &cy);
			SetPosition(cx, cy, (int)tab->GetCanvas()->GetFloor());
			SetZoom(tab->GetCanvas()->GetZoom());
		}
	}

	int GetMapX() {
		wxSize size = GetClientSize();
		int width = size.GetWidth() > 0 ? size.GetWidth() : 400;
		return (view_x + (width * (double)zoom) / 2.0) / 32;
	}

	int GetMapY() {
		wxSize size = GetClientSize();
		int height = size.GetHeight() > 0 ? size.GetHeight() : 300;
		return (view_y + (height * (double)zoom) / 2.0) / 32;
	}

	int GetClientWidth() const override {
		return client_w;
	}
	int GetClientHeight() const override {
		return client_h;
	}

private:
	int view_x;
	int view_y;

	int client_w = ClientMapWidth;
	int client_h = ClientMapHeight;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MapPreviewCanvas, MapCanvas)
EVT_MOTION(MapPreviewCanvas::OnMouseMove)
EVT_MOUSEWHEEL(MapPreviewCanvas::OnWheel)
EVT_ENTER_WINDOW(MapPreviewCanvas::OnGainMouse)
EVT_LEAVE_WINDOW(MapPreviewCanvas::OnLoseMouse)
EVT_LEFT_DOWN(MapPreviewCanvas::OnMouseLeftClick)
EVT_LEFT_UP(MapPreviewCanvas::OnMouseLeftRelease)
EVT_RIGHT_DOWN(MapPreviewCanvas::OnMouseRightClick)
EVT_RIGHT_UP(MapPreviewCanvas::OnMouseRightRelease)
EVT_MIDDLE_DOWN(MapPreviewCanvas::OnMouseCenterClick)
EVT_MIDDLE_UP(MapPreviewCanvas::OnMouseCenterRelease)
EVT_PAINT(MapCanvas::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(LuaDialog, wxDialog)
EVT_CLOSE(LuaDialog::OnClose)
END_EVENT_TABLE()

LuaDialog::LuaDialog(const std::string& title, sol::this_state ts) :
	wxDialog(g_gui.root, wxID_ANY, wxString(title), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	lua(ts) {
	createLayout();
}

// Overload constructor to handle options
LuaDialog::LuaDialog(sol::table options, sol::this_state ts) :
	wxDialog(g_gui.root, wxID_ANY, wxString(options.get_or("title", "Script Dialog"s)), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | (options.get_or("resizable", true) ? wxRESIZE_BORDER : 0) | (options.get_or("topmost", false) ? wxSTAY_ON_TOP : 0)),
	lua(ts) {

	reqWidth = options.get_or("width", -1);
	reqHeight = options.get_or("height", -1);
	reqX = options.get_or("x", -1);
	reqY = options.get_or("y", -1);

	if (reqWidth != -1 || reqHeight != -1) {
		SetMinSize(wxSize(reqWidth != -1 ? reqWidth : 150, reqHeight != -1 ? reqHeight : 100));
		SetSize(reqX != -1 ? reqX : -1, reqY != -1 ? reqY : -1, reqWidth, reqHeight);
	} else if (reqX != -1 || reqY != -1) {
		SetPosition(wxPoint(reqX != -1 ? reqX : GetPosition().x, reqY != -1 ? reqY : GetPosition().y));
	}

	if (options.get_or(std::string("dockable"), false)) {
		dockPanel = new wxPanel(g_gui.root, wxID_ANY);

		wxAuiPaneInfo info;
		std::string title = options.get_or(std::string("title"), "Script Dialog"s);
		std::string id = options.get_or(std::string("id"), title);

		info.Name(id);
		info.Caption(title);
		info.Right().Layer(1).Position(1).CloseButton(true).MaximizeButton(true);

		int minW = options.get_or("min_width", reqWidth != -1 ? reqWidth : 200);
		int minH = options.get_or("min_height", reqHeight != -1 ? reqHeight : 150);
		info.MinSize(wxSize(minW, minH));

		if (reqWidth != -1 || reqHeight != -1) {
			info.BestSize(wxSize(reqWidth != -1 ? reqWidth : 300, reqHeight != -1 ? reqHeight : 200));
		} else {
			info.BestSize(wxSize(300, 200));
		}

		info.Dockable(true).Floatable(true);

		g_gui.aui_manager->AddPane(dockPanel, info);
		g_gui.aui_manager->Update();
	}

	if (options["onclose"].valid()) {
		oncloseCallback = options["onclose"];
	}

	createLayout();
}

LuaDialog::~LuaDialog() {
	while (hotkeySuspendCount > 0) {
		resumeHotkeys();
	}
	if (dockPanel) {
		g_gui.aui_manager->DetachPane(dockPanel);
		g_gui.aui_manager->Update();
		dockPanel->Destroy();
		dockPanel = nullptr;
	}
}

void LuaDialog::createLayout() {
	mainSizer = new wxBoxSizer(wxVERTICAL);
	if (dockPanel) {
		dockPanel->SetSizer(mainSizer);
	} else {
		SetSizer(mainSizer);
	}
	sizerStack.push(mainSizer);
}

void LuaDialog::ensureRowSizer() {
	if (!currentRowSizer) {
		currentRowSizer = new wxBoxSizer(wxHORIZONTAL);
		getSizerForWidget()->Add(currentRowSizer, 0, wxEXPAND | wxALL, 5);
	}
}

void LuaDialog::finishCurrentRow() {
	currentRowSizer = nullptr;
}

wxWindow* LuaDialog::getParentForWidget() {
	if (currentTabPanel) {
		return currentTabPanel;
	}
	if (dockPanel) {
		return dockPanel;
	}
	return this;
}

wxSizer* LuaDialog::getSizerForWidget() {
	if (!sizerStack.empty()) {
		return sizerStack.top();
	}
	// Fallback mechanism (should shouldn't happen if managed correctly)
	if (currentTabSizer) {
		return currentTabSizer;
	}
	return mainSizer;
}

LuaDialog* LuaDialog::wrap(sol::table options) {
	finishCurrentRow();

	// 'wrap' is just a horizontal box sizer that wraps?
	// Wx doesn't have a simple WrapSizer like that, it has wxWrapSizer.
	// But usually "visual grouping side-by-side" is just a Horizontal Box.

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	getSizerForWidget()->Add(sizer, 1, wxEXPAND | wxALL, 0); // Add to current
	sizerStack.push(sizer); // Push to stack

	return this;
}

LuaDialog* LuaDialog::endwrap() {
	finishCurrentRow();
	if (!sizerStack.empty() && sizerStack.top() != mainSizer && (!currentTabSizer || sizerStack.top() != currentTabSizer)) {
		sizerStack.pop();
	}
	return this;
}

LuaDialog* LuaDialog::box(sol::table options) {
	finishCurrentRow();

	std::string orient = options.get_or("orient", "vertical"s);
	std::string label = options.get_or("label", ""s);

	wxSizer* sizer;
	if (!label.empty()) {
		// Static box sizer
		wxStaticBoxSizer* staticBox = new wxStaticBoxSizer(wxVERTICAL, getParentForWidget(), wxString(label));
		if (orient == "horizontal") {
			// wxStaticBoxSizer constructor takes orient, but we passed vertical
			// Actually can check if we can change it or construct differently
			// Reconstruct correct orient
			delete staticBox;
			staticBox = new wxStaticBoxSizer(wxHORIZONTAL, getParentForWidget(), wxString(label));
		}
		sizer = staticBox;
	} else {
		// Normal box sizer
		if (orient == "horizontal") {
			sizer = new wxBoxSizer(wxHORIZONTAL);
		} else {
			sizer = new wxBoxSizer(wxVERTICAL);
		}
	}

	getSizerForWidget()->Add(sizer, 1, wxEXPAND | wxALL, 5);
	sizerStack.push(sizer);

	return this;
}

LuaDialog* LuaDialog::endbox() {
	finishCurrentRow();
	if (!sizerStack.empty() && sizerStack.top() != mainSizer && (!currentTabSizer || sizerStack.top() != currentTabSizer)) {
		sizerStack.pop();
	}
	return this;
}

LuaDialog* LuaDialog::label(sol::table options) {
	ensureRowSizer();

	std::string text = options.get_or("text", "label"s);
	std::string id = options.get_or("id", ""s);

	wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(text));
	currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	if (!id.empty()) {
		LuaDialogWidget widget;
		widget.id = id;
		widget.type = "label";
		widget.widget = label;
		widgets.push_back(widget);
	}

	applyCommonOptions(label, options);

	return this;
}

LuaDialog* LuaDialog::mapCanvas(sol::table options) {
	ensureRowSizer();

	Editor* editor = g_gui.GetCurrentEditor();
	if (!editor) {
		wxMessageBox("You must have a map open to use the Map Canvas widget.", "Error", wxOK | wxICON_ERROR);
		return this;
	}

	std::string id = options.get_or("id", "map_c_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	// Create the canvas
	MapPreviewCanvas* canvas = new MapPreviewCanvas(getParentForWidget(), *editor);

	// Smaller default size to keep the window compact
	canvas->SetMinSize(wxSize(200, 150));

	// Default to center of map start
	canvas->SetPosition(1000, 1000, 7);

	currentRowSizer->Add(canvas, 1, wxEXPAND | wxALL, 0);

	if (!id.empty()) {
		LuaDialogWidget widget;
		widget.id = id;
		widget.type = "mapCanvas";
		widget.widget = canvas;
		widgets.push_back(widget);
	}

	applyCommonOptions(canvas, options);
	return this;
}

LuaDialog* LuaDialog::input(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "input_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	std::string text = options.get_or("text", ""s);
	bool focus = options.get_or("focus", false);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxTextCtrl* input = new wxTextCtrl(getParentForWidget(), wxID_ANY, wxString(text), wxDefaultPosition, wxSize(150, -1));
	currentRowSizer->Add(input, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	input->Bind(wxEVT_SET_FOCUS, [this](wxFocusEvent& event) {
		suspendHotkeys();
		event.Skip();
	});
	input->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event) {
		resumeHotkeys();
		event.Skip();
	});

	if (focus) {
		input->SetFocus();
	}

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "input";
	widget.widget = input;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	// Store initial value
	values[id] = sol::make_object(lua, text);

	// Bind change event
	input->Bind(wxEVT_TEXT, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(input, options);
	return this;
}

LuaDialog* LuaDialog::number(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or(std::string("id"), "number_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or(std::string("label"), ""s);
	double value = options.get_or(std::string("text"), options.get_or(std::string("value"), 0.0));
	int decimals = options.get_or(std::string("decimals"), 0);
	double minVal = options.get_or(std::string("min"), -999999.0);
	double maxVal = options.get_or(std::string("max"), 999999.0);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxSpinCtrlDouble* spin = new wxSpinCtrlDouble(getParentForWidget(), wxID_ANY, "", wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS, minVal, maxVal, value, decimals == 0 ? 1 : std::pow(0.1, decimals));
	spin->SetDigits(decimals);
	currentRowSizer->Add(spin, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "number";
	widget.widget = spin;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, value);

	spin->Bind(wxEVT_SPINCTRLDOUBLE, [this, id](wxSpinDoubleEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(spin, options);
	return this;
}

LuaDialog* LuaDialog::slider(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "slider_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	int value = options.get_or("value", 0);
	int minVal = options.get_or("min", 0);
	int maxVal = options.get_or("max", 100);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxSlider* slider = new wxSlider(getParentForWidget(), wxID_ANY, value, minVal, maxVal, wxDefaultPosition, wxSize(150, -1));
	currentRowSizer->Add(slider, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "slider";
	widget.widget = slider;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, value);

	slider->Bind(wxEVT_SLIDER, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(slider, options);
	return this;
}

LuaDialog* LuaDialog::check(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "check_"s + std::to_string(widgets.size()));
	std::string text = options.get_or("text", ""s);
	bool selected = options.get_or("selected", false);

	wxCheckBox* checkbox = new wxCheckBox(getParentForWidget(), wxID_ANY, wxString(text));
	checkbox->SetValue(selected);
	currentRowSizer->Add(checkbox, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "check";
	widget.widget = checkbox;
	if (options["onclick"].valid()) {
		widget.onclick = options["onclick"];
	}
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, selected);

	checkbox->Bind(wxEVT_CHECKBOX, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(checkbox, options);
	return this;
}

LuaDialog* LuaDialog::radio(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "radio_"s + std::to_string(widgets.size()));
	std::string text = options.get_or("text", ""s);
	bool selected = options.get_or("selected", false);

	wxRadioButton* radio = new wxRadioButton(getParentForWidget(), wxID_ANY, wxString(text));
	radio->SetValue(selected);
	currentRowSizer->Add(radio, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "radio";
	widget.widget = radio;
	if (options["onclick"].valid()) {
		widget.onclick = options["onclick"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, selected);

	radio->Bind(wxEVT_RADIOBUTTON, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(radio, options);
	return this;
}

LuaDialog* LuaDialog::combobox(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "combobox_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	std::string selected = options.get_or("option", ""s);

	wxArrayString choices;
	if (options["options"].valid()) {
		sol::table opts = options["options"];
		for (size_t i = 1; i <= opts.size(); ++i) {
			if (opts[i].valid()) {
				choices.Add(wxString(opts[i].get<std::string>()));
			}
		}
	}

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxChoice* choice = new wxChoice(getParentForWidget(), wxID_ANY, wxDefaultPosition, wxSize(150, -1), choices);

	if (!selected.empty()) {
		int idx = choice->FindString(wxString(selected));
		if (idx != wxNOT_FOUND) {
			choice->SetSelection(idx);
		}
	} else if (choices.GetCount() > 0) {
		choice->SetSelection(0);
		selected = choices[0].ToStdString();
	}

	currentRowSizer->Add(choice, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "combobox";
	widget.widget = choice;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, selected);

	choice->Bind(wxEVT_CHOICE, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(choice, options);
	return this;
}

LuaDialog* LuaDialog::button(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "button_"s + std::to_string(widgets.size()));
	std::string text = options.get_or("text", "Button"s);
	bool focus = options.get_or("focus", false);

	wxButton* btn = new wxButton(getParentForWidget(), wxID_ANY, wxString(text));
	currentRowSizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	if (focus) {
		btn->SetDefault();
	}

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "button";
	widget.widget = btn;
	if (options["onclick"].valid()) {
		widget.onclick = options["onclick"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, false);

	btn->Bind(wxEVT_BUTTON, [this, id](wxCommandEvent&) {
		onButtonClick(id);
	});

	applyCommonOptions(btn, options);
	return this;
}

LuaDialog* LuaDialog::color(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "color_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);

	wxColour defaultColor = *wxBLACK;
	if (options["color"].valid()) {
		sol::table c = options["color"];
		int r = c.get_or("red", c.get_or(1, 0));
		int g = c.get_or("green", c.get_or(2, 0));
		int b = c.get_or("blue", c.get_or(3, 0));
		defaultColor = wxColour(r, g, b);
	}

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxColourPickerCtrl* picker = new wxColourPickerCtrl(getParentForWidget(), wxID_ANY, defaultColor);
	currentRowSizer->Add(picker, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "color";
	widget.widget = picker;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	// Store color as table
	sol::table colorTable = lua.create_table();
	colorTable["red"] = defaultColor.Red();
	colorTable["green"] = defaultColor.Green();
	colorTable["blue"] = defaultColor.Blue();
	values[id] = colorTable;

	picker->Bind(wxEVT_COLOURPICKER_CHANGED, [this, id](wxColourPickerEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(picker, options);
	return this;
}

///////////////////////////////////////////////////////////////////////////////
// LuaDialogListBox - Custom list widget for LuaDialog
///////////////////////////////////////////////////////////////////////////////

struct LuaListBoxItem {
	std::string text;
	int iconId = 0;
	LuaAPI::LuaImage customImage;
	std::string tooltip;
};

class LuaDialogListBox : public wxVListBox {
public:
	LuaDialogListBox(wxWindow* parent, wxWindowID id, const wxSize& size) :
		wxVListBox(parent, id, wxDefaultPosition, size, wxLB_SINGLE) {
		Bind(wxEVT_LEFT_DOWN, &LuaDialogListBox::OnLeftDown, this);
		Bind(wxEVT_LEFT_DCLICK, &LuaDialogListBox::OnLeftDouble, this);
		Bind(wxEVT_MOTION, &LuaDialogListBox::OnMotion, this);
		Bind(wxEVT_LEAVE_WINDOW, &LuaDialogListBox::OnLeaveWindow, this);
	}

	void AddItem(const std::string& text, int iconId, const LuaAPI::LuaImage& img, const std::string& tooltip) {
		LuaListBoxItem item;
		item.text = text;
		item.iconId = iconId;
		item.customImage = img;
		item.tooltip = tooltip;
		if (!item.customImage.isValid() && iconId > 0) {
			item.customImage = LuaAPI::LuaImage::loadFromSprite(iconId);
		}
		items.push_back(item);
		SetItemCount(items.size());
	}

	void Clear() {
		items.clear();
		SetItemCount(0);
		lastTooltipItem = -1;
	}

	void SetIconSize(int width, int height) {
		iconWidth = width;
		iconHeight = height;
		if (itemHeight < iconHeight + 6) {
			itemHeight = iconHeight + 6;
		}
	}

	void SetItemHeight(int height) {
		itemHeight = height;
	}

	void SetShowText(bool value) {
		showText = value;
	}

	void SetSmooth(bool value) {
		smooth = value;
	}

	const LuaListBoxItem* GetItem(int index) const {
		if (index < 0 || index >= static_cast<int>(items.size())) {
			return nullptr;
		}
		return &items[index];
	}

	void OnLeftDown(wxMouseEvent& event) {
		int n = HitTest(event.GetPosition());
		if (n != wxNOT_FOUND) {
			SetSelection(n);
			CallAfter([this, n]() {
				wxCommandEvent ce(wxEVT_LISTBOX, GetId());
				ce.SetInt(n);
				ce.SetEventObject(this);
				ProcessWindowEvent(ce);
			});
		}
		event.Skip();
	}

	void OnLeftDouble(wxMouseEvent& event) {
		int n = HitTest(event.GetPosition());
		if (n != wxNOT_FOUND) {
			SetSelection(n);
			SetFocus();
			CallAfter([this, n]() {
				wxCommandEvent ce(wxEVT_LISTBOX_DCLICK, GetId());
				ce.SetInt(n);
				ce.SetEventObject(this);
				ProcessWindowEvent(ce);
			});
		}
		event.Skip();
	}

	void OnMotion(wxMouseEvent& event) {
		int n = HitTest(event.GetPosition());
		if (n != wxNOT_FOUND && n < static_cast<int>(items.size())) {
			if (n != lastTooltipItem) {
				const std::string& tip = items[n].tooltip;
				if (!tip.empty()) {
					SetToolTip(wxString(tip));
				} else {
					UnsetToolTip();
				}
				lastTooltipItem = n;
			}
		} else if (lastTooltipItem != -1) {
			UnsetToolTip();
			lastTooltipItem = -1;
		}
		event.Skip();
	}

	void OnLeaveWindow(wxMouseEvent& event) {
		UnsetToolTip();
		lastTooltipItem = -1;
		event.Skip();
	}

	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override {
		if (n >= items.size()) {
			return;
		}
		const LuaListBoxItem& item = items[n];

		bool isSelected = IsSelected(n);
		if (isSelected) {
			dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
		} else {
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.SetPen(*wxTRANSPARENT_PEN);
		}
		dc.DrawRectangle(rect);

		// Icon
		int textX = rect.x + 5;
		if (item.customImage.isValid()) {
			wxBitmap bmp = item.customImage.getBitmap(iconWidth, iconHeight, smooth);
			if (bmp.IsOk()) {
				dc.DrawBitmap(bmp, rect.x + 2, rect.y + (rect.height - iconHeight) / 2, true);
				textX += iconWidth + 4;
			}
		}

		if (showText) {
			if (isSelected) {
				dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
			} else {
				dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
			}

			wxString text = wxString(item.text);
			wxSize extent = dc.GetTextExtent(text);
			dc.DrawText(text, textX, rect.y + (rect.height - extent.y) / 2);
		}
	}

	virtual wxCoord OnMeasureItem(size_t n) const override {
		return itemHeight;
	}

	int iconWidth = 16;
	int iconHeight = 16;
	int itemHeight = 24;
	bool showText = true;
	bool smooth = true;
	int lastTooltipItem = -1;
	std::vector<LuaListBoxItem> items;
};

// Specialized ListCtrl for Grid with Tooltips
class LuaGridCtrl : public wxListCtrl {
public:
	LuaGridCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
		wxListCtrl(parent, id, pos, size, style) {
		Bind(wxEVT_MOTION, &LuaGridCtrl::OnMotion, this);
		Bind(wxEVT_LEAVE_WINDOW, &LuaGridCtrl::OnLeaveWindow, this);
	}

	std::map<long, std::string> tooltips;
	long lastTooltipItem = -1;
	int iconWidth = 32;
	int iconHeight = 32;
	int cellWidth = -1;
	int cellHeight = -1;

	void AddTooltip(long item, const std::string& tip) {
		tooltips[item] = tip;
	}

	void ClearTooltips() {
		tooltips.clear();
		lastTooltipItem = -1;
	}

	std::string GetTooltip(long item) const {
		auto it = tooltips.find(item);
		if (it == tooltips.end()) {
			return ""s;
		}
		return it->second;
	}

	void OnMotion(wxMouseEvent& event) {
		int flags = 0;
		long index = HitTest(event.GetPosition(), flags);
		if (index != wxNOT_FOUND) {
			if (index != lastTooltipItem) {
				if (tooltips.count(index)) {
					SetToolTip(wxString(tooltips[index]));
				} else {
					UnsetToolTip();
				}
				lastTooltipItem = index;
			}
		} else {
			if (lastTooltipItem != -1) {
				UnsetToolTip();
				lastTooltipItem = -1;
			}
		}
		event.Skip();
	}

	void OnLeaveWindow(wxMouseEvent& event) {
		UnsetToolTip();
		lastTooltipItem = -1;
		event.Skip();
	}
};

LuaDialog* LuaDialog::list(sol::table options) {
	finishCurrentRow();

	std::string id = options.get_or(std::string("id"), "list_"s + std::to_string(widgets.size()));
	int width = options.get_or(std::string("width"), 200);
	int height = options.get_or(std::string("height"), 150);
	int iconWidth = options.get_or(std::string("icon_width"), 16);
	int iconHeight = options.get_or(std::string("icon_height"), 16);
	int iconSize = options.get_or(std::string("icon_size"), -1);
	int itemHeight = options.get_or(std::string("item_height"), 24);
	bool showText = options.get_or(std::string("show_text"), true);
	bool smooth = options.get_or(std::string("smooth"), true);

	if (iconSize > 0) {
		iconWidth = iconSize;
		iconHeight = iconSize;
	}

	LuaDialogListBox* listbox = new LuaDialogListBox(getParentForWidget(), wxID_ANY, wxSize(width, height));
	listbox->SetIconSize(iconWidth, iconHeight);
	if (itemHeight > 0) {
		listbox->SetItemHeight(itemHeight);
	}
	listbox->SetShowText(showText);
	listbox->SetSmooth(smooth);
	getSizerForWidget()->Add(listbox, 1, wxEXPAND | wxALL, 5);

	// Populate items
	if (options["items"].valid()) {
		sol::table itemsTable = options["items"];
		for (auto& pair : itemsTable) {
			if (pair.second.is<sol::table>()) {
				sol::table itemTable = pair.second;
				std::string text = itemTable.get_or("text", ""s);
				int icon = itemTable.get_or("icon", 0);
				std::string tooltip = itemTable.get_or("tooltip", ""s);
				LuaAPI::LuaImage img;
				if (itemTable["image"].valid()) {
					if (itemTable["image"].is<LuaAPI::LuaImage>()) {
						img = itemTable["image"].get<LuaAPI::LuaImage>();
					} else if (itemTable["image"].is<std::string>()) {
						img = LuaAPI::LuaImage::loadFromFile(itemTable["image"].get<std::string>());
					}
				}
				listbox->AddItem(text, icon, img, tooltip);
			}
		}
	}

	// Selection
	int selection = options.get_or("selection", 0);
	if (selection > 0 && selection <= (int)listbox->items.size()) {
		listbox->SetSelection(selection - 1);
	}

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "list";
	widget.widget = listbox;

	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	if (options["ondoubleclick"].valid()) {
		widget.ondoubleclick = options["ondoubleclick"];
	}
	if (options["oncontextmenu"].valid()) {
		widget.oncontextmenu = options["oncontextmenu"];
	}
	if (options["onleftclick"].valid()) {
		widget.onleftclick = options["onleftclick"];
	}
	if (options["onrightclick"].valid()) {
		widget.onrightclick = options["onrightclick"];
	}
	widgets.push_back(widget);

	// Bind selection event
	listbox->Bind(wxEVT_LISTBOX, [this, id](wxCommandEvent&) {
		onWidgetChange(id);
	});

	// Bind double click event
	listbox->Bind(wxEVT_LISTBOX_DCLICK, [this, id](wxCommandEvent&) {
		onWidgetDoubleClick(id);
	});

	sol::function listLeftClick = widget.onleftclick;
	listbox->Bind(wxEVT_LEFT_DOWN, [this, id, listbox, listLeftClick](wxMouseEvent& event) {
		if (!listLeftClick.valid() || !lua.lua_state() || !listLeftClick.lua_state()) {
			event.Skip();
			return;
		}
		int index = listbox->HitTest(event.GetPosition());
		if (index == wxNOT_FOUND) {
			event.Skip();
			return;
		}
		const LuaListBoxItem* item = listbox->GetItem(index);
		if (!item) {
			event.Skip();
			return;
		}
		listbox->SetSelection(index);
		listbox->SetFocus();
		values[id] = sol::make_object(lua, index + 1);
		sol::table info = lua.create_table();
		info["type"] = "list";
		info["index"] = index + 1;
		info["text"] = item->text;
		if (!item->tooltip.empty()) {
			info["tooltip"] = item->tooltip;
		}
		info["widget_id"] = id;
		try {
			listLeftClick(this, info);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
		event.Skip();
	});

	sol::function listRightClick = widget.onrightclick;
	listbox->Bind(wxEVT_RIGHT_DOWN, [this, id, listbox, listRightClick](wxMouseEvent& event) {
		if (!listRightClick.valid() || !lua.lua_state() || !listRightClick.lua_state()) {
			event.Skip();
			return;
		}
		int index = listbox->HitTest(event.GetPosition());
		if (index == wxNOT_FOUND) {
			event.Skip();
			return;
		}
		const LuaListBoxItem* item = listbox->GetItem(index);
		if (!item) {
			event.Skip();
			return;
		}
		listbox->SetSelection(index);
		listbox->SetFocus();
		values[id] = sol::make_object(lua, index + 1);
		sol::table info = lua.create_table();
		info["type"] = "list";
		info["index"] = index + 1;
		info["text"] = item->text;
		if (!item->tooltip.empty()) {
			info["tooltip"] = item->tooltip;
		}
		info["widget_id"] = id;
		try {
			listRightClick(this, info);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
		event.Skip();
	});

	// Bind context menu event
	sol::function listContextMenu = widget.oncontextmenu;
	listbox->Bind(wxEVT_CONTEXT_MENU, [this, id, listbox, listContextMenu](wxContextMenuEvent& event) {
		if (!listContextMenu.valid() || !lua.lua_state() || !listContextMenu.lua_state()) {
			return;
		}
		wxPoint screenPos = event.GetPosition();
		if (screenPos == wxDefaultPosition) {
			screenPos = wxGetMousePosition();
		}
		wxPoint clientPos = listbox->ScreenToClient(screenPos);
		int index = listbox->HitTest(clientPos);
		if (index == wxNOT_FOUND) {
			return;
		}

		const LuaListBoxItem* item = listbox->GetItem(index);
		if (!item) {
			return;
		}

		sol::table info = lua.create_table();
		info["type"] = "list";
		info["index"] = index + 1;
		info["text"] = item->text;
		if (!item->tooltip.empty()) {
			info["tooltip"] = item->tooltip;
		}
		info["widget_id"] = id;
		popupContextMenu(listContextMenu, info, listbox, screenPos);
	});

	applyCommonOptions(listbox, options);
	return this;
}

LuaDialog* LuaDialog::file(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or(std::string("id"), "file_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or(std::string("label"), ""s);
	std::string filename = options.get_or(std::string("filename"), ""s);
	std::string filetypes = options.get_or(std::string("filetypes"), "*.*"s);
	bool save = options.get_or(std::string("save"), false);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxFilePickerCtrl* picker;
	if (save) {
		picker = new wxFilePickerCtrl(getParentForWidget(), wxID_ANY, wxString(filename), "Select a file", wxString(filetypes), wxDefaultPosition, wxSize(200, -1), wxFLP_SAVE | wxFLP_USE_TEXTCTRL);
	} else {
		picker = new wxFilePickerCtrl(getParentForWidget(), wxID_ANY, wxString(filename), "Select a file", wxString(filetypes), wxDefaultPosition, wxSize(200, -1), wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_USE_TEXTCTRL);
	}
	currentRowSizer->Add(picker, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "file";
	widget.widget = picker;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, filename);

	picker->Bind(wxEVT_FILEPICKER_CHANGED, [this, id](wxFileDirPickerEvent&) {
		onWidgetChange(id);
	});

	applyCommonOptions(picker, options);
	return this;
}

LuaDialog* LuaDialog::image(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "image_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	int width = options.get_or("width", -1);
	int height = options.get_or("height", -1);
	bool smooth = options.get_or("smooth", true); // Default to smooth scaling

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	wxBitmap bmp;
	LuaAPI::LuaImage luaImage;

	// Check for different image sources
	if (options["image"].valid()) {
		// Direct LuaImage object
		luaImage = options["image"].get<LuaAPI::LuaImage>();
	} else if (options["path"].valid()) {
		// Load from file path
		std::string path = options.get<std::string>("path");
		luaImage = LuaAPI::LuaImage::loadFromFile(path);
	} else if (options["itemid"].valid()) {
		// Load from item sprite ID
		int itemId = options.get<int>("itemid");
		luaImage = LuaAPI::LuaImage::loadFromItemSprite(itemId);
	} else if (options["spriteid"].valid()) {
		// Load from raw sprite ID
		int spriteId = options.get<int>("spriteid");
		luaImage = LuaAPI::LuaImage::loadFromSprite(spriteId);
	}

	// Get bitmap with optional resizing
	if (luaImage.isValid()) {
		if (width > 0 && height > 0) {
			bmp = luaImage.getBitmap(width, height, smooth);
		} else if (width > 0) {
			// Scale proportionally based on width
			double factor = static_cast<double>(width) / luaImage.getWidth();
			int newHeight = static_cast<int>(luaImage.getHeight() * factor);
			bmp = luaImage.getBitmap(width, newHeight, smooth);
		} else if (height > 0) {
			// Scale proportionally based on height
			double factor = static_cast<double>(height) / luaImage.getHeight();
			int newWidth = static_cast<int>(luaImage.getWidth() * factor);
			bmp = luaImage.getBitmap(newWidth, height, smooth);
		} else {
			bmp = luaImage.getBitmap();
		}
	}

	// Create placeholder if no valid image
	if (!bmp.IsOk()) {
		int placeholderW = width > 0 ? width : 32;
		int placeholderH = height > 0 ? height : 32;
		bmp.Create(placeholderW, placeholderH);
		wxMemoryDC dc(bmp);
		dc.SetBrush(*wxLIGHT_GREY_BRUSH);
		dc.SetPen(*wxGREY_PEN);
		dc.DrawRectangle(0, 0, placeholderW, placeholderH);
		dc.SelectObject(wxNullBitmap);
	}

	wxStaticBitmap* staticBmp = new wxStaticBitmap(getParentForWidget(), wxID_ANY, bmp);
	currentRowSizer->Add(staticBmp, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "image";
	widget.widget = staticBmp;
	widgets.push_back(widget);

	// Store the LuaImage in values for later retrieval/modification
	values[id] = sol::make_object(lua, luaImage);

	applyCommonOptions(staticBmp, options);
	return this;
}

LuaDialog* LuaDialog::item(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "item_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	int itemId = options.get_or("itemid", 0);

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	int clientId = (itemId > 100 && itemId <= g_items.getMaxID()) ? g_items[itemId].clientID : 0;
	ItemButton* btn = new ItemButton(getParentForWidget(), RENDER_SIZE_32x32, clientId);
	currentRowSizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "item";
	widget.widget = btn;
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	if (options["onclick"].valid()) {
		widget.onclick = options["onclick"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, itemId);

	bool readonly = options.get_or("readonly", false);

	// Bind click event
	btn->Bind(wxEVT_BUTTON, [this, id, btn, readonly](wxCommandEvent&) {
		// Check if we have a custom click handler
		bool handled = false;
		for (auto& w : widgets) {
			if (w.id == id && w.onclick.valid()) {
				onButtonClick(id);
				handled = true;
				break;
			}
		}

		if (!handled && !readonly) {
			FindItemDialog dlg(this, "Select Item", false);
			if (dlg.ShowModal() == wxID_OK) {
				int newItemId = dlg.getResultID();
				int newClientId = (newItemId > 100 && newItemId <= g_items.getMaxID()) ? g_items[newItemId].clientID : 0;
				btn->SetSprite(newClientId);
				btn->Refresh(); // Ensure visual update holding the new sprite
				values[id] = sol::make_object(lua, newItemId);
				onWidgetChange(id);
			}
		}
	});

	applyCommonOptions(btn, options);
	return this;
}

LuaDialog* LuaDialog::separator() {
	finishCurrentRow();

	wxStaticLine* line = new wxStaticLine(getParentForWidget(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	getSizerForWidget()->Add(line, 0, wxEXPAND | wxALL, 5);

	// Separator doesn't support many options but we can try
	// applyCommonOptions(line, options); // options not passed to separator() usually
	// But signature is LuaDialog* separator(); so no options table.

	return this;
}

LuaDialog* LuaDialog::newrow() {
	finishCurrentRow();
	return this;
}

LuaDialog* LuaDialog::tab(sol::table options) {
	finishCurrentRow();

	std::string id = options.get_or("id", "tab_"s + std::to_string(widgets.size()));
	std::string text = options.get_or("text", "Tab"s);
	bool isButton = options.get_or("button", false) || options.get_or("is_button", false);
	int insertIndex = options.get_or("index", -1);
	sol::function onclick;
	sol::function oncontextmenu;
	if (options["onclick"].valid()) {
		onclick = options["onclick"];
	}
	if (options["oncontextmenu"].valid()) {
		oncontextmenu = options["oncontextmenu"];
	}

	// Create notebook if needed
	if (!currentNotebook) {
		wxWindow* parent = dockPanel ? static_cast<wxWindow*>(dockPanel) : static_cast<wxWindow*>(this);
		currentNotebook = new wxNotebook(parent, wxID_ANY);
		mainSizer->Add(currentNotebook, 1, wxEXPAND | wxALL, 5);
		activeNotebook = currentNotebook;
		tabInfos.clear();
		notebookEventsBound = false;
	}

	if (!notebookEventsBound) {
		notebookEventsBound = true;
		currentNotebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGING, [this](wxNotebookEvent& event) {
			int newSelection = event.GetSelection();
			if (newSelection != wxNOT_FOUND && newSelection < static_cast<int>(tabInfos.size())) {
				if (tabInfos[newSelection].isButton) {
					if (suppressTabButtonClick && suppressTabButtonIndex == newSelection) {
						suppressTabButtonClick = false;
						suppressTabButtonIndex = -1;
						event.Veto();
						return;
					}
					CallAfter([this, newSelection]() {
						handleTabButtonClick(newSelection);
					});
					event.Veto();
					return;
				}
			}
			event.Skip();
		});

		currentNotebook->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event) {
			if (!activeNotebook || tabInfos.empty()) {
				event.Skip();
				return;
			}
			wxPoint pos = event.GetPosition();
			long flags = 0;
			int index = activeNotebook->HitTest(pos, &flags);
			if (index != wxNOT_FOUND && index < static_cast<int>(tabInfos.size())) {
				if (tabInfos[index].isButton) {
					suppressTabButtonClick = true;
					suppressTabButtonIndex = index;
					CallAfter([this, index]() {
						handleTabButtonClick(index);
					});
					CallAfter([this, index]() {
						if (suppressTabButtonClick && suppressTabButtonIndex == index) {
							suppressTabButtonClick = false;
							suppressTabButtonIndex = -1;
						}
					});
					event.StopPropagation();
					return;
				}
			}
			event.Skip();
		});

		currentNotebook->Bind(wxEVT_CONTEXT_MENU, [this](wxContextMenuEvent& event) {
			if (!activeNotebook || tabInfos.empty()) {
				return;
			}
			wxPoint screenPos = event.GetPosition();
			if (screenPos == wxDefaultPosition) {
				screenPos = wxGetMousePosition();
			}
			wxPoint clientPos = activeNotebook->ScreenToClient(screenPos);
			long flags = 0;
			int index = activeNotebook->HitTest(clientPos, &flags);
			if (index == wxNOT_FOUND || index >= static_cast<int>(tabInfos.size())) {
				return;
			}
			handleTabContextMenu(index, screenPos);
		});
	}

	// Create new tab panel
	currentTabPanel = new wxPanel(currentNotebook);
	currentTabSizer = new wxBoxSizer(wxVERTICAL);
	currentTabPanel->SetSizer(currentTabSizer);
	int pageIndex = -1;
	if (insertIndex > 0) {
		int zeroBased = insertIndex - 1;
		int pageCount = currentNotebook->GetPageCount();
		if (zeroBased < 0) {
			zeroBased = 0;
		} else if (zeroBased > pageCount) {
			zeroBased = pageCount;
		}
		currentNotebook->InsertPage(zeroBased, currentTabPanel, wxString(text));
		pageIndex = zeroBased;
	} else {
		currentNotebook->AddPage(currentTabPanel, wxString(text));
		pageIndex = currentNotebook->GetPageCount() - 1;
	}

	LuaDialogTab tabInfo;
	tabInfo.id = id;
	tabInfo.text = text;
	tabInfo.isButton = isButton;
	tabInfo.onclick = onclick;
	tabInfo.oncontextmenu = oncontextmenu;
	if (pageIndex >= 0 && pageIndex <= static_cast<int>(tabInfos.size())) {
		tabInfos.insert(tabInfos.begin() + pageIndex, tabInfo);
	} else {
		tabInfos.push_back(tabInfo);
	}

	sizerStack.push(currentTabSizer);
	tabSizers.push_back(currentTabSizer);

	return this;
}

LuaDialog* LuaDialog::endtabs() {
	finishCurrentRow();
	// Pop all tab sizers that were added
	while (!tabSizers.empty()) {
		wxSizer* topSizer = tabSizers.back();
		if (!sizerStack.empty() && sizerStack.top() == topSizer) {
			sizerStack.pop();
		}
		tabSizers.pop_back();
	}

	currentNotebook = nullptr;
	currentTabPanel = nullptr;
	currentTabSizer = nullptr;
	return this;
}

sol::table LuaDialog::makeTabInfoTable(int index) {
	sol::table info = lua.create_table();
	if (index < 0 || index >= static_cast<int>(tabInfos.size())) {
		return info;
	}
	const LuaDialogTab& tab = tabInfos[index];
	info["index"] = index + 1;
	info["text"] = tab.text;
	info["id"] = tab.id;
	info["is_button"] = tab.isButton;
	return info;
}

void LuaDialog::handleTabButtonClick(int index) {
	if (index < 0 || index >= static_cast<int>(tabInfos.size())) {
		return;
	}
	sol::function onclick = tabInfos[index].onclick;
	if (!onclick.valid()) {
		return;
	}
	if (!lua.lua_state()) {
		return;
	}
	if (!onclick.lua_state()) {
		return;
	}
	sol::table info = makeTabInfoTable(index);
	try {
		onclick(this, info);
	} catch (const sol::error& e) {
		wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
	}
}

void LuaDialog::handleTabContextMenu(int index, const wxPoint& screenPos) {
	if (index < 0 || index >= static_cast<int>(tabInfos.size())) {
		return;
	}
	sol::function oncontextmenu = tabInfos[index].oncontextmenu;
	if (!oncontextmenu.valid()) {
		return;
	}
	if (!lua.lua_state()) {
		return;
	}
	if (!oncontextmenu.lua_state()) {
		return;
	}

	sol::table info = makeTabInfoTable(index);
	info["type"] = "tab";
	popupContextMenu(oncontextmenu, info, activeNotebook, screenPos);
}

void LuaDialog::popupContextMenu(const sol::function& callback, sol::table info, wxWindow* window, const wxPoint& screenPos) {
	if (!callback.valid()) {
		return;
	}
	if (!lua.lua_state() || !callback.lua_state()) {
		return;
	}

	sol::object result;
	try {
		result = callback(this, info);
	} catch (const sol::error& e) {
		wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		return;
	}

	if (!result.valid() || !result.is<sol::table>()) {
		return;
	}

	sol::table items = result.as<sol::table>();
	wxMenu menu;
	std::map<int, sol::function> callbacks;
	int nextId = wxID_HIGHEST + 1;

	for (auto& pair : items) {
		if (!pair.second.is<sol::table>()) {
			continue;
		}
		sol::table item = pair.second.as<sol::table>();
		bool separator = item.get_or(std::string("separator"), false);
		std::string text = item.get_or(std::string("text"), ""s);

		if (separator || text.empty()) {
			menu.AppendSeparator();
			continue;
		}

		int id = nextId++;
		wxMenuItem* menuItem = menu.Append(id, wxString(text));
		bool enabled = item.get_or(std::string("enabled"), true);
		menuItem->Enable(enabled);

		if (item["onclick"].valid()) {
			sol::function fn = item["onclick"];
			if (fn.lua_state()) {
				callbacks[id] = fn;
			}
		}
	}

	if (menu.GetMenuItemCount() == 0) {
		return;
	}

	menu.Bind(wxEVT_MENU, [this, callbacks, info](wxCommandEvent& event) mutable {
		if (!lua.lua_state()) {
			return;
		}
		auto it = callbacks.find(event.GetId());
		if (it == callbacks.end()) {
			return;
		}
		if (!it->second.lua_state()) {
			return;
		}
		try {
			it->second(this, info);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
	});

	if (!window) {
		return;
	}
	wxPoint clientPos = window->ScreenToClient(screenPos);
	window->PopupMenu(&menu, clientPos);
}

LuaDialog* LuaDialog::show(sol::optional<sol::table> options) {
	if (dockPanel) {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(dockPanel);
		if (info.IsOk()) {
			info.Show();
			g_gui.aui_manager->Update();
		}
		isShowing = true;
		return this;
	}

	// Fit the dialog first to establish baseline size from sizers
	Fit();

	// Re-apply requested size from constructor if it's larger than fitted size
	if (reqWidth != -1 || reqHeight != -1) {
		wxSize sz = GetSize();
		if (reqWidth != -1) {
			sz.x = std::max(sz.x, reqWidth);
		}
		if (reqHeight != -1) {
			sz.y = std::max(sz.y, reqHeight);
		}
		SetSize(sz);
	}

	// Process options
	if (options) {
		sol::table opts = *options;
		waitMode = opts.get_or("wait", true);

		if (opts["bounds"].valid()) {
			setBounds(opts["bounds"]);
		} else if (reqX == -1 && reqY == -1) {
			bool wantsCenter = true;
			std::string centerOn = "parent";
			if (opts["center"].valid()) {
				if (opts["center"].is<bool>()) {
					wantsCenter = opts.get<bool>("center");
				} else if (opts["center"].is<std::string>()) {
					centerOn = opts.get<std::string>("center");
				}
			}
			if (opts["center_on"].valid() && opts["center_on"].is<std::string>()) {
				centerOn = opts.get<std::string>("center_on");
			}
			if (wantsCenter) {
				if (centerOn == "screen") {
					Centre();
				} else {
					CentreOnParent();
				}
			}
		}
	} else {
		if (reqX == -1 && reqY == -1) {
			CentreOnParent();
		}
	}

	isShowing = true;

	if (waitMode) {
		ShowModal();
		isShowing = false;
	} else {
		Show();
	}

	return this;
}

void LuaDialog::close() {
	while (hotkeySuspendCount > 0) {
		resumeHotkeys();
	}
	if (dockPanel) {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(dockPanel);
		if (info.IsOk()) {
			info.Hide();
			g_gui.aui_manager->Update();
		}
		isShowing = false;
		if (oncloseCallback.valid()) {
			try {
				oncloseCallback();
			} catch (const sol::error& e) {
				wxMessageBox(wxString("Script error in onclose: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
			}
		}
		return;
	}

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		Hide();
	}
	isShowing = false;
	if (oncloseCallback.valid()) {
		try {
			oncloseCallback();
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error in onclose: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
	}
}

void LuaDialog::OnClose(wxCloseEvent& event) {
	if (IsModal()) {
		EndModal(wxID_CANCEL);
	} else {
		Hide();
	}
	isShowing = false;

	if (oncloseCallback.valid()) {
		try {
			oncloseCallback();
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error in onclose: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
	}

	if (!IsModal() && event.CanVeto()) {
		// Prevent destruction of modeless dialog so it can be reused
		// event.Veto() is not needed if we don't Skip(), but it's safer
		event.Veto();
		return;
	}
	event.Skip();
}

LuaDialog* LuaDialog::modify(sol::table options) {
	// Modify widget properties
	for (auto& pair : options) {
		std::string id = pair.first.as<std::string>();
		sol::table props = pair.second.as<sol::table>();

		for (auto& widget : widgets) {
			if (widget.id == id) {
				// Apply property changes based on widget type
				if (widget.type == "mapCanvas") {
					MapPreviewCanvas* canvas = static_cast<MapPreviewCanvas*>(widget.widget);
					if (props["x"].valid() && props["y"].valid()) {
						canvas->SetPosition(props.get_or(std::string("x"), 1000), props.get_or(std::string("y"), 1000), props.get_or(std::string("z"), (int)canvas->GetFloor()));
					}
					if (props["zoom"].valid()) {
						canvas->SetZoom(props.get_or(std::string("zoom"), 1.0));
					}
					if (props["floor"].valid()) {
						canvas->SetFloor(props.get_or(std::string("floor"), 7));
						canvas->Refresh();
					}
					if (props["sync"].valid() && props.get_or(std::string("sync"), false)) {
						canvas->SyncView();
					}
					if (props["client_w"].valid() && props["client_h"].valid()) {
						canvas->SetViewSize(props.get_or(std::string("client_w"), ClientMapWidth), props.get_or(std::string("client_h"), ClientMapHeight));
					}
					if (props["light"].valid()) {
						canvas->SetLight(props.get_or(std::string("light"), false));
					}
				} else if (widget.type == "input") {
					wxTextCtrl* ctrl = static_cast<wxTextCtrl*>(widget.widget);
					if (props["text"].valid()) {
						ctrl->SetValue(wxString(props.get_or("text", ""s)));
					}
				} else if (widget.type == "number") {
					wxSpinCtrlDouble* ctrl = static_cast<wxSpinCtrlDouble*>(widget.widget);
					if (props["value"].valid()) {
						ctrl->SetValue(props.get_or("value", 0.0));
					}
				} else if (widget.type == "check") {
					wxCheckBox* ctrl = static_cast<wxCheckBox*>(widget.widget);
					if (props["selected"].valid()) {
						ctrl->SetValue(props.get_or("selected", false));
					}
				} else if (widget.type == "item") {
					ItemButton* ctrl = static_cast<ItemButton*>(widget.widget);
					if (props["itemid"].valid()) {
						int newItemId = props.get_or("itemid", 0);
						ctrl->SetSprite(newItemId);
						ctrl->Refresh();
					}
				} else if (widget.type == "label") {
					wxStaticText* ctrl = static_cast<wxStaticText*>(widget.widget);
					if (props["text"].valid()) {
						ctrl->SetLabel(wxString(props.get_or("text", ""s)));
					}
				} else if (widget.type == "list") {
					LuaDialogListBox* ctrl = static_cast<LuaDialogListBox*>(widget.widget);
					if (props["icon_width"].valid() || props["icon_height"].valid() || props["icon_size"].valid()) {
						int iconWidth = props.get_or("icon_width", ctrl->iconWidth);
						int iconHeight = props.get_or("icon_height", ctrl->iconHeight);
						int iconSize = props.get_or("icon_size", -1);
						if (iconSize > 0) {
							iconWidth = iconSize;
							iconHeight = iconSize;
						}
						ctrl->SetIconSize(iconWidth, iconHeight);
					}
					if (props["item_height"].valid()) {
						ctrl->SetItemHeight(props.get_or("item_height", ctrl->itemHeight));
					}
					if (props["show_text"].valid()) {
						ctrl->SetShowText(props.get_or("show_text", true));
					}
					if (props["smooth"].valid()) {
						ctrl->SetSmooth(props.get_or("smooth", true));
					}
					if (props["items"].valid()) {
						ctrl->Freeze();
						ctrl->Clear();
						sol::table itemsTable = props["items"];
						for (auto& pair : itemsTable) {
							if (pair.second.is<sol::table>()) {
								sol::table itemTable = pair.second;
								std::string text = itemTable.get_or("text", ""s);
								int icon = itemTable.get_or("icon", 0);
								std::string tooltip = itemTable.get_or("tooltip", ""s);
								LuaAPI::LuaImage img;
								if (itemTable["image"].valid()) {
									if (itemTable["image"].is<LuaAPI::LuaImage>()) {
										img = itemTable["image"].get<LuaAPI::LuaImage>();
									} else if (itemTable["image"].is<std::string>()) {
										img = LuaAPI::LuaImage::loadFromFile(itemTable["image"].get<std::string>());
									}
								}
								ctrl->AddItem(text, icon, img, tooltip);
							}
						}
						ctrl->Refresh();
						ctrl->Thaw();
					}
					if (props["selection"].valid()) {
						int selection = props.get_or("selection", 0);
						if (selection > 0 && selection <= (int)ctrl->items.size()) {
							ctrl->SetSelection(selection - 1);
						} else {
							ctrl->SetSelection(wxNOT_FOUND);
						}
					}

				} else if (widget.type == "grid") {
					LuaGridCtrl* ctrl = static_cast<LuaGridCtrl*>(widget.widget);
					int iconWidth = ctrl->iconWidth;
					int iconHeight = ctrl->iconHeight;
					int cellWidth = ctrl->cellWidth;
					int cellHeight = ctrl->cellHeight;
					bool updateIconSize = false;
					bool updateCellSize = false;

					int iconSize = props.get_or("icon_size", -1);
					int itemSize = props.get_or("item_size", -1);
					int itemWidth = props.get_or("item_width", -1);
					int itemHeight = props.get_or("item_height", -1);
					int iconWidthOpt = props.get_or("icon_width", -1);
					int iconHeightOpt = props.get_or("icon_height", -1);
					int cellSize = props.get_or("cell_size", -1);
					int cellWidthOpt = props.get_or("cell_width", -1);
					int cellHeightOpt = props.get_or("cell_height", -1);

					if (iconSize > 0) {
						iconWidth = iconSize;
						iconHeight = iconSize;
						updateIconSize = true;
					}
					if (itemSize > 0) {
						iconWidth = itemSize;
						iconHeight = itemSize;
						updateIconSize = true;
					}
					if (itemWidth > 0) {
						iconWidth = itemWidth;
						updateIconSize = true;
					}
					if (itemHeight > 0) {
						iconHeight = itemHeight;
						updateIconSize = true;
					}
					if (iconWidthOpt > 0) {
						iconWidth = iconWidthOpt;
						updateIconSize = true;
					}
					if (iconHeightOpt > 0) {
						iconHeight = iconHeightOpt;
						updateIconSize = true;
					}

					if (cellSize > 0) {
						cellWidth = cellSize;
						cellHeight = cellSize;
						updateCellSize = true;
					}
					if (cellWidthOpt > 0) {
						cellWidth = cellWidthOpt;
						updateCellSize = true;
					}
					if (cellHeightOpt > 0) {
						cellHeight = cellHeightOpt;
						updateCellSize = true;
					}

					if (updateCellSize) {
						if (cellWidth <= 0) {
							cellWidth = iconWidth + 8;
						}
						if (cellHeight <= 0) {
							cellHeight = iconHeight + 8;
						}
#ifdef __WXMSW__
						ListView_SetIconSpacing(static_cast<HWND>(ctrl->GetHandle()), cellWidth, cellHeight);
#endif
						ctrl->cellWidth = cellWidth;
						ctrl->cellHeight = cellHeight;
					}
					if (props["selection"].valid()) {
						int selection = props.get_or("selection", 0);
						long item = -1;
						while ((item = ctrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) {
							ctrl->SetItemState(item, 0, wxLIST_STATE_SELECTED);
						}
						if (selection > 0 && selection <= ctrl->GetItemCount()) {
							ctrl->SetItemState(selection - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
						}
					}

					if (props["items"].valid()) {
						ctrl->Freeze();
						ctrl->DeleteAllItems();
						ctrl->ClearTooltips();
						wxImageList* imageList = ctrl->GetImageList(wxIMAGE_LIST_NORMAL);
						if (!imageList || updateIconSize) {
							imageList = new wxImageList(iconWidth, iconHeight, true);
							ctrl->AssignImageList(imageList, wxIMAGE_LIST_NORMAL);
							ctrl->iconWidth = iconWidth;
							ctrl->iconHeight = iconHeight;
						} else {
							imageList->RemoveAll();
						}

						sol::table itemsTable = props["items"];
						for (auto& pair : itemsTable) {
							if (pair.second.is<sol::table>()) {
								sol::table itemTable = pair.second;
								std::string text = itemTable.get_or("text", ""s);

								LuaAPI::LuaImage img;
								int imageIndex = -1;

								if (itemTable["image"].valid()) {
									if (itemTable["image"].is<LuaAPI::LuaImage>()) {
										img = itemTable["image"].get<LuaAPI::LuaImage>();
									} else if (itemTable["image"].is<std::string>()) {
										img = LuaAPI::LuaImage::loadFromFile(itemTable["image"].get<std::string>());
									}
								}

								if (img.isValid()) {
									imageIndex = imageList->Add(img.getBitmap(iconWidth, iconHeight));
								}

								long index = ctrl->InsertItem(ctrl->GetItemCount(), text, imageIndex);
								ctrl->SetItemData(index, index + 1);

								std::string tooltip = itemTable.get_or("tooltip", ""s);
								if (!tooltip.empty()) {
									// Must match the type cast
									LuaGridCtrl* gridCtrl = static_cast<LuaGridCtrl*>(ctrl);
									gridCtrl->AddTooltip(index, tooltip);
								}
							}
						}
						ctrl->Thaw();
					}
				} else if (widget.type == "image") {
					wxStaticBitmap* ctrl = static_cast<wxStaticBitmap*>(widget.widget);

					LuaAPI::LuaImage luaImage;
					int width = props.get_or("width", -1);
					int height = props.get_or("height", -1);
					bool smooth = props.get_or("smooth", true);

					if (props["image"].valid()) {
						luaImage = props["image"].get<LuaAPI::LuaImage>();
					} else if (props["path"].valid()) {
						std::string path = props.get<std::string>("path");
						luaImage = LuaAPI::LuaImage::loadFromFile(path);
					} else if (props["itemid"].valid()) {
						int itemId = props.get<int>("itemid");
						luaImage = LuaAPI::LuaImage::loadFromItemSprite(itemId);
					} else if (props["spriteid"].valid()) {
						int spriteId = props.get<int>("spriteid");
						luaImage = LuaAPI::LuaImage::loadFromSprite(spriteId);
					}

					if (luaImage.isValid()) {
						wxBitmap bmp;
						if (width > 0 && height > 0) {
							bmp = luaImage.getBitmap(width, height, smooth);
						} else if (width > 0) {
							double factor = static_cast<double>(width) / luaImage.getWidth();
							int newHeight = static_cast<int>(luaImage.getHeight() * factor);
							bmp = luaImage.getBitmap(width, newHeight, smooth);
						} else if (height > 0) {
							double factor = static_cast<double>(height) / luaImage.getHeight();
							int newWidth = static_cast<int>(luaImage.getWidth() * factor);
							bmp = luaImage.getBitmap(newWidth, height, smooth);
						} else {
							bmp = luaImage.getBitmap();
						}
						if (bmp.IsOk()) {
							ctrl->SetBitmap(bmp);
							values[id] = sol::make_object(lua, luaImage);
						}
					}
				}

				applyCommonOptions(widget.widget, props);
				break;
			}
		}
	}
	return this;
}

LuaDialog* LuaDialog::grid(sol::table options) {
	ensureRowSizer();

	std::string id = options.get_or("id", "grid_"s + std::to_string(widgets.size()));
	std::string labelText = options.get_or("label", ""s);
	int iconWidth = options.get_or("icon_width", 32);
	int iconHeight = options.get_or("icon_height", 32);
	int iconSize = options.get_or("icon_size", -1);
	int itemSize = options.get_or("item_size", -1);
	int itemWidth = options.get_or("item_width", -1);
	int itemHeight = options.get_or("item_height", -1);
	int cellSize = options.get_or("cell_size", -1);
	int cellWidth = options.get_or("cell_width", -1);
	int cellHeight = options.get_or("cell_height", -1);
	bool labelWrap = options.get_or("label_wrap", true);
	bool showText = options.get_or("show_text", true);

	if (iconSize > 0) {
		iconWidth = iconSize;
		iconHeight = iconSize;
	}
	if (itemSize > 0) {
		iconWidth = itemSize;
		iconHeight = itemSize;
	}
	if (itemWidth > 0) {
		iconWidth = itemWidth;
	}
	if (itemHeight > 0) {
		iconHeight = itemHeight;
	}
	if (iconWidth <= 0) {
		iconWidth = 32;
	}
	if (iconHeight <= 0) {
		iconHeight = 32;
	}
	if (cellSize > 0) {
		cellWidth = cellSize;
		cellHeight = cellSize;
	}

	if (!labelText.empty()) {
		wxStaticText* label = new wxStaticText(getParentForWidget(), wxID_ANY, wxString(labelText));
		currentRowSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	}

	// Use LuaGridCtrl in Icon mode for grid view
	LuaGridCtrl* grid = new LuaGridCtrl(getParentForWidget(), wxID_ANY, wxDefaultPosition, wxSize(300, 200), wxLC_ICON | wxLC_AUTOARRANGE | wxLC_SINGLE_SEL);

	// Create an image list
	wxImageList* imageList = new wxImageList(iconWidth, iconHeight, true);
	grid->AssignImageList(imageList, wxIMAGE_LIST_NORMAL);

	if (!showText) {
		wxFont font = grid->GetFont();
		font.SetPointSize(1);
		grid->SetFont(font);
	}

#ifdef __WXMSW__
	if (!labelWrap) {
		HWND hwnd = static_cast<HWND>(grid->GetHandle());
		LONG_PTR style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
		style |= LVS_NOLABELWRAP;
		::SetWindowLongPtr(hwnd, GWL_STYLE, style);
		::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	}

	if (cellWidth > 0 || cellHeight > 0) {
		if (cellWidth <= 0) {
			cellWidth = iconWidth + 8;
		}
		if (cellHeight <= 0) {
			cellHeight = iconHeight + 8;
		}
		ListView_SetIconSpacing(static_cast<HWND>(grid->GetHandle()), cellWidth, cellHeight);
	}
#endif
	grid->iconWidth = iconWidth;
	grid->iconHeight = iconHeight;
	grid->cellWidth = cellWidth;
	grid->cellHeight = cellHeight;

	if (options["items"].valid()) {
		sol::table itemsTable = options["items"];
		for (auto& pair : itemsTable) {
			if (pair.second.is<sol::table>()) {
				sol::table itemTable = pair.second;
				std::string text = itemTable.get_or("text", ""s);
				if (!showText) {
					text.clear();
				}

				LuaAPI::LuaImage img;
				int imageIndex = -1;

				if (itemTable["image"].valid()) {
					if (itemTable["image"].is<LuaAPI::LuaImage>()) {
						img = itemTable["image"].get<LuaAPI::LuaImage>();
					} else if (itemTable["image"].is<std::string>()) {
						img = LuaAPI::LuaImage::loadFromFile(itemTable["image"].get<std::string>());
					}
				}

				if (img.isValid()) {
					imageIndex = imageList->Add(img.getBitmap(iconWidth, iconHeight));
				}

				long index = grid->InsertItem(grid->GetItemCount(), text, imageIndex);
				// Store original item index/id in data if needed, or rely on position
				grid->SetItemData(index, index + 1); // 1-based index

				std::string tooltip = itemTable.get_or("tooltip", ""s);
				if (!tooltip.empty()) {
					grid->AddTooltip(index, tooltip);
				}
			}
		}
	}

	currentRowSizer->Add(grid, 1, wxEXPAND | wxALL, 0);

	LuaDialogWidget widget;
	widget.id = id;
	widget.type = "grid";
	widget.widget = grid;
	if (options["ondoubleclick"].valid()) {
		widget.ondoubleclick = options["ondoubleclick"];
	}
	if (options["onchange"].valid()) {
		widget.onchange = options["onchange"];
	}
	if (options["onleftclick"].valid()) {
		widget.onleftclick = options["onleftclick"];
	}
	if (options["onrightclick"].valid()) {
		widget.onrightclick = options["onrightclick"];
	}
	if (options["oncontextmenu"].valid()) {
		widget.oncontextmenu = options["oncontextmenu"];
	}
	widgets.push_back(widget);

	values[id] = sol::make_object(lua, 0);

	// Bind events
	grid->Bind(wxEVT_LIST_ITEM_SELECTED, [this, id, grid](wxListEvent& event) {
		values[id] = sol::make_object(lua, event.GetIndex() + 1);
		onWidgetChange(id);
	});

	grid->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this, id, grid](wxListEvent& event) {
		values[id] = sol::make_object(lua, event.GetIndex() + 1);
		onWidgetDoubleClick(id);
	});

	sol::function gridLeftClick = widget.onleftclick;
	grid->Bind(wxEVT_LEFT_DOWN, [this, id, grid, gridLeftClick](wxMouseEvent& event) {
		if (!gridLeftClick.valid() || !lua.lua_state() || !gridLeftClick.lua_state()) {
			event.Skip();
			return;
		}
		int flags = 0;
		long index = grid->HitTest(event.GetPosition(), flags);
		if (index == wxNOT_FOUND) {
			event.Skip();
			return;
		}
		grid->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		grid->SetFocus();
		values[id] = sol::make_object(lua, static_cast<int>(index + 1));
		sol::table info = lua.create_table();
		info["type"] = "grid";
		info["index"] = static_cast<int>(index + 1);
		info["text"] = grid->GetItemText(index).ToStdString();
		std::string tooltip = static_cast<LuaGridCtrl*>(grid)->GetTooltip(index);
		if (!tooltip.empty()) {
			info["tooltip"] = tooltip;
		}
		info["widget_id"] = id;
		try {
			gridLeftClick(this, info);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
		event.Skip();
	});

	sol::function gridRightClick = widget.onrightclick;
	grid->Bind(wxEVT_RIGHT_DOWN, [this, id, grid, gridRightClick](wxMouseEvent& event) {
		if (!gridRightClick.valid() || !lua.lua_state() || !gridRightClick.lua_state()) {
			event.Skip();
			return;
		}
		int flags = 0;
		long index = grid->HitTest(event.GetPosition(), flags);
		if (index == wxNOT_FOUND) {
			event.Skip();
			return;
		}
		grid->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		grid->SetFocus();
		values[id] = sol::make_object(lua, static_cast<int>(index + 1));
		sol::table info = lua.create_table();
		info["type"] = "grid";
		info["index"] = static_cast<int>(index + 1);
		info["text"] = grid->GetItemText(index).ToStdString();
		std::string tooltip = static_cast<LuaGridCtrl*>(grid)->GetTooltip(index);
		if (!tooltip.empty()) {
			info["tooltip"] = tooltip;
		}
		info["widget_id"] = id;
		try {
			gridRightClick(this, info);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
		event.Skip();
	});

	sol::function gridContextMenu = widget.oncontextmenu;
	grid->Bind(wxEVT_CONTEXT_MENU, [this, id, grid, gridContextMenu](wxContextMenuEvent& event) {
		if (!gridContextMenu.valid() || !lua.lua_state() || !gridContextMenu.lua_state()) {
			return;
		}
		wxPoint screenPos = event.GetPosition();
		if (screenPos == wxDefaultPosition) {
			screenPos = wxGetMousePosition();
		}
		wxPoint clientPos = grid->ScreenToClient(screenPos);
		int flags = 0;
		long index = grid->HitTest(clientPos, flags);
		if (index == wxNOT_FOUND) {
			return;
		}

		sol::table info = lua.create_table();
		info["type"] = "grid";
		info["index"] = static_cast<int>(index + 1);
		info["text"] = grid->GetItemText(index).ToStdString();
		std::string tooltip = static_cast<LuaGridCtrl*>(grid)->GetTooltip(index);
		if (!tooltip.empty()) {
			info["tooltip"] = tooltip;
		}
		info["widget_id"] = id;
		popupContextMenu(gridContextMenu, info, grid, screenPos);
	});

	applyCommonOptions(grid, options);
	return this;
}

void LuaDialog::repaint() {
	Refresh();
	Update();
}

void LuaDialog::clear() {
	// Clear all widgets
	widgets.clear();
	values.clear();

	// Destroy all children of mainSizer
	mainSizer->Clear(true); // true = delete windows

	// Reset layout state
	while (!sizerStack.empty()) {
		sizerStack.pop();
	}
	sizerStack.push(mainSizer);
	tabSizers.clear();

	currentRowSizer = nullptr;
	currentNotebook = nullptr;
	activeNotebook = nullptr;
	currentTabPanel = nullptr;
	currentTabSizer = nullptr;
	tabInfos.clear();
	notebookEventsBound = false;
	suppressTabButtonClick = false;
	suppressTabButtonIndex = -1;
	dockPanel ? dockPanel->Layout() : Layout();
}

void LuaDialog::layout() {
	dockPanel ? dockPanel->Layout() : Layout();
}

sol::table LuaDialog::getData() {
	collectAllValues();

	sol::table data = lua.create_table();
	for (auto& pair : values) {
		data[pair.first] = pair.second;
	}
	return data;
}

void LuaDialog::setData(sol::table data) {
	for (auto& pair : data) {
		std::string id = pair.first.as<std::string>();
		values[id] = pair.second;
	}
}

sol::table LuaDialog::getBounds() {
	sol::table bounds = lua.create_table();
	wxRect rect = GetRect();
	bounds["x"] = rect.x;
	bounds["y"] = rect.y;
	bounds["width"] = rect.width;
	bounds["height"] = rect.height;
	return bounds;
}

sol::object LuaDialog::getActiveTab() {
	wxNotebook* notebook = activeNotebook ? activeNotebook : currentNotebook;
	if (!notebook) {
		return sol::make_object(lua, sol::nil);
	}

	int selection = notebook->GetSelection();
	if (selection == wxNOT_FOUND) {
		return sol::make_object(lua, sol::nil);
	}

	wxString text = notebook->GetPageText(selection);
	return sol::make_object(lua, text.ToStdString());
}

void LuaDialog::setBounds(sol::table bounds) {
	int x = bounds.get_or("x", GetPosition().x);
	int y = bounds.get_or("y", GetPosition().y);
	int w = bounds.get_or("width", GetSize().GetWidth());
	int h = bounds.get_or("height", GetSize().GetHeight());
	SetSize(x, y, w, h);
}

void LuaDialog::updateValue(const std::string& id) {
	for (auto& widget : widgets) {
		if (widget.id == id) {
			if (widget.type == "input") {
				wxTextCtrl* ctrl = static_cast<wxTextCtrl*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetValue().ToStdString());
			} else if (widget.type == "number") {
				wxSpinCtrlDouble* ctrl = static_cast<wxSpinCtrlDouble*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetValue());
			} else if (widget.type == "slider") {
				wxSlider* ctrl = static_cast<wxSlider*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetValue());
			} else if (widget.type == "check") {
				wxCheckBox* ctrl = static_cast<wxCheckBox*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetValue());
			} else if (widget.type == "radio") {
				wxRadioButton* ctrl = static_cast<wxRadioButton*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetValue());
			} else if (widget.type == "combobox") {
				wxChoice* ctrl = static_cast<wxChoice*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetStringSelection().ToStdString());
			} else if (widget.type == "color") {
				wxColourPickerCtrl* ctrl = static_cast<wxColourPickerCtrl*>(widget.widget);
				wxColour c = ctrl->GetColour();
				sol::table colorTable = lua.create_table();
				colorTable["red"] = c.Red();
				colorTable["green"] = c.Green();
				colorTable["blue"] = c.Blue();
				values[id] = colorTable;
			} else if (widget.type == "file") {
				wxFilePickerCtrl* ctrl = static_cast<wxFilePickerCtrl*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetPath().ToStdString());
			} else if (widget.type == "item") {
				// Value is updated in the event handler
			} else if (widget.type == "mapCanvas") {
				MapPreviewCanvas* ctrl = static_cast<MapPreviewCanvas*>(widget.widget);
				sol::table mapData = lua.create_table();
				mapData["x"] = ctrl->GetMapX();
				mapData["y"] = ctrl->GetMapY();
				mapData["z"] = ctrl->GetFloor();
				mapData["zoom"] = ctrl->GetZoom();
				values[id] = mapData;
			} else if (widget.type == "list") {
				LuaDialogListBox* ctrl = static_cast<LuaDialogListBox*>(widget.widget);
				values[id] = sol::make_object(lua, ctrl->GetSelection() + 1); // 1-based for Lua
			} else if (widget.type == "grid") {
				wxListCtrl* ctrl = static_cast<wxListCtrl*>(widget.widget);
				long item = -1;
				item = ctrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				if (item != -1) {
					values[id] = sol::make_object(lua, item + 1);
				} else {
					values[id] = sol::make_object(lua, 0);
				}
			}
			break;
		}
	}
}

void LuaDialog::collectAllValues() {
	for (auto& widget : widgets) {
		updateValue(widget.id);
	}
}

void LuaDialog::applyCommonOptions(wxWindow* widget, sol::table options) {
	if (options["tooltip"].valid()) {
		widget->SetToolTip(wxString(options.get<std::string>("tooltip")));
	}
	if (options["enabled"].valid()) {
		widget->Enable(options.get<bool>("enabled"));
	}
	if (options["visible"].valid()) {
		widget->Show(options.get<bool>("visible"));
	}
}

void LuaDialog::suspendHotkeys() {
	if (hotkeySuspendCount == 0) {
		if (g_gui.AreHotkeysEnabled()) {
			g_gui.DisableHotkeys();
			hotkeysDisabledByDialog = true;
		}
	}
	++hotkeySuspendCount;
}

void LuaDialog::resumeHotkeys() {
	if (hotkeySuspendCount <= 0) {
		hotkeySuspendCount = 0;
		return;
	}
	--hotkeySuspendCount;
	if (hotkeySuspendCount == 0 && hotkeysDisabledByDialog) {
		g_gui.EnableHotkeys();
		hotkeysDisabledByDialog = false;
	}
}

void LuaDialog::onWidgetChange(const std::string& id) {
	updateValue(id);

	// Call onchange callback if set
	sol::function onchange;
	for (auto& widget : widgets) {
		if (widget.id == id && widget.onchange.valid()) {
			onchange = widget.onchange;
			break;
		}
	}

	if (onchange.valid()) {
		try {
			onchange(this);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
	}
}

void LuaDialog::onButtonClick(const std::string& id) {
	// Set button value to true (for detecting which button was clicked)
	values[id] = sol::make_object(lua, true);

	// Call onclick callback if set
	sol::function onclick;
	for (auto& widget : widgets) {
		if (widget.id == id && widget.onclick.valid()) {
			onclick = widget.onclick;
			break;
		}
	}

	if (onclick.valid()) {
		try {
			onclick(this);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
		return; // onclick handles closing if needed
	}

	// Default behavior: close dialog on button click (for OK/Cancel buttons)
	std::string lowerId = id;
	std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), ::tolower);
	if (lowerId == "ok" || lowerId == "cancel" || lowerId == "close") {
		close();
	}
}

void LuaDialog::onWidgetDoubleClick(const std::string& id) {
	updateValue(id);

	sol::function ondoubleclick;
	for (auto& widget : widgets) {
		if (widget.id == id && widget.ondoubleclick.valid()) {
			ondoubleclick = widget.ondoubleclick;
			break;
		}
	}

	if (ondoubleclick.valid()) {
		try {
			ondoubleclick(this);
		} catch (const sol::error& e) {
			wxMessageBox(wxString("Script error: ") + e.what(), "Lua Error", wxOK | wxICON_ERROR);
		}
	}
}

namespace LuaAPI {

	void registerDialog(sol::state& lua) {
		// Register Dialog class with constructor and method chaining
		lua.new_usertype<LuaDialog>("Dialog",
									// Constructor: Dialog{title = "...", topmost = true}
									sol::call_constructor, sol::factories([](sol::table options, sol::this_state ts) { return new LuaDialog(options, ts); }, [](const std::string& title, sol::this_state ts) { return new LuaDialog(title, ts); }),

									// Widget methods (return self for chaining)
									"label", &LuaDialog::label, "input", &LuaDialog::input, "number", &LuaDialog::number, "slider", &LuaDialog::slider, "check", &LuaDialog::check, "radio", &LuaDialog::radio, "combobox", &LuaDialog::combobox, "button", &LuaDialog::button, "color", &LuaDialog::color, "item", &LuaDialog::item, "file", &LuaDialog::file, "image", &LuaDialog::image, "separator", &LuaDialog::separator, "newrow", &LuaDialog::newrow, "tab", &LuaDialog::tab, "endtabs", &LuaDialog::endtabs, "wrap", &LuaDialog::wrap, "endwrap", &LuaDialog::endwrap, "box", &LuaDialog::box, "endbox", &LuaDialog::endbox, "mapCanvas", &LuaDialog::mapCanvas, "list", &LuaDialog::list, "grid", &LuaDialog::grid,

									// Dialog control
									"show", &LuaDialog::show, "close", &LuaDialog::close, "modify", &LuaDialog::modify, "repaint", &LuaDialog::repaint, "clear", &LuaDialog::clear, "layout", &LuaDialog::layout,

									// Properties
									"data", sol::property(&LuaDialog::getData, &LuaDialog::setData), "bounds", sol::property(&LuaDialog::getBounds, &LuaDialog::setBounds), "dockable", sol::property(&LuaDialog::isDockable), "activeTab", sol::property(&LuaDialog::getActiveTab));
	}

} // namespace LuaAPI
