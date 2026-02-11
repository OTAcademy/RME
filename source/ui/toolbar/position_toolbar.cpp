//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/position_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "editor/editor.h"
#include "util/image_manager.h"
#include <wx/artprov.h>

const wxString PositionToolBar::PANE_NAME = "position_toolbar";

PositionToolBar::PositionToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	wxBitmap go_bitmap = IMAGE_MANAGER.GetBitmap(ICON_LOCATION_ARROW, icon_size);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_POSITION, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORZ_TEXT);
	toolbar->SetToolBitmapSize(icon_size);

	x_control = newd NumberTextCtrl(toolbar, wxID_ANY, 0, 0, MAP_MAX_WIDTH, wxTE_PROCESS_ENTER, "X", wxDefaultPosition, FROM_DIP(parent, wxSize(60, 20)));
	x_control->SetToolTip("X Coordinate");

	y_control = newd NumberTextCtrl(toolbar, wxID_ANY, 0, 0, MAP_MAX_HEIGHT, wxTE_PROCESS_ENTER, "Y", wxDefaultPosition, FROM_DIP(parent, wxSize(60, 20)));
	y_control->SetToolTip("Y Coordinate");

	z_control = newd NumberTextCtrl(toolbar, wxID_ANY, 0, 0, MAP_MAX_LAYER, wxTE_PROCESS_ENTER, "Z", wxDefaultPosition, FROM_DIP(parent, wxSize(35, 20)));
	z_control->SetToolTip("Z Coordinate");

	go_button = newd wxButton(toolbar, TOOLBAR_POSITION_GO, wxEmptyString, wxDefaultPosition, parent->FromDIP(wxSize(22, 20)));
	go_button->SetBitmap(go_bitmap);
	go_button->SetToolTip("Go To Position");

	toolbar->AddControl(x_control);
	toolbar->AddControl(y_control);
	toolbar->AddControl(z_control);
	toolbar->AddControl(go_button);
	toolbar->Realize();

	x_control->Bind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	x_control->Bind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	y_control->Bind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	y_control->Bind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	z_control->Bind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	z_control->Bind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	go_button->Bind(wxEVT_BUTTON, &PositionToolBar::OnGoClick, this);
}

PositionToolBar::~PositionToolBar() {
	x_control->Unbind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	x_control->Unbind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	y_control->Unbind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	y_control->Unbind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	z_control->Unbind(wxEVT_TEXT_PASTE, &PositionToolBar::OnPaste, this);
	z_control->Unbind(wxEVT_KEY_UP, &PositionToolBar::OnKeyUp, this);
	go_button->Unbind(wxEVT_BUTTON, &PositionToolBar::OnGoClick, this);
}

void PositionToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	bool has_map = editor != nullptr;

	toolbar->EnableTool(TOOLBAR_POSITION_GO, has_map);
	x_control->Enable(has_map);
	y_control->Enable(has_map);
	z_control->Enable(has_map);

	if (has_map) {
		x_control->SetMaxValue(editor->map.getWidth());
		y_control->SetMaxValue(editor->map.getHeight());
	}
	toolbar->Refresh();
}

void PositionToolBar::OnGoClick(wxCommandEvent& event) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	if (event.GetId() == TOOLBAR_POSITION_GO) {
		Position pos(x_control->GetIntValue(), y_control->GetIntValue(), z_control->GetIntValue());
		if (pos.isValid()) {
			g_gui.SetScreenCenterPosition(pos);
		}
	}
}

void PositionToolBar::OnKeyUp(wxKeyEvent& event) {
	if (event.GetKeyCode() == WXK_TAB) {
		if (x_control->HasFocus()) {
			y_control->SelectAll();
			y_control->SetFocus();
		} else if (y_control->HasFocus()) {
			z_control->SelectAll();
			z_control->SetFocus();
		} else if (z_control->HasFocus()) {
			go_button->SetFocus();
		}
	} else if (event.GetKeyCode() == WXK_NUMPAD_ENTER || event.GetKeyCode() == WXK_RETURN) {
		Position pos(x_control->GetIntValue(), y_control->GetIntValue(), z_control->GetIntValue());
		if (pos.isValid()) {
			g_gui.SetScreenCenterPosition(pos);
		}
	}
	event.Skip();
}

void PositionToolBar::OnPaste(wxClipboardTextEvent& event) {
	Position position;
	const Map& currentMap = g_gui.GetCurrentMap();
	if (posFromClipboard(position, currentMap.getWidth(), currentMap.getHeight())) {
		x_control->SetIntValue(position.x);
		y_control->SetIntValue(position.y);
		z_control->SetIntValue(position.z);
	} else {
		event.Skip();
	}
}
