//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/standard_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "ui/main_menubar.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "util/image_manager.h"
#include <wx/artprov.h>

const wxString StandardToolBar::PANE_NAME = "standard_toolbar";

StandardToolBar::StandardToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	wxBitmap new_bitmap = IMAGE_MANAGER.GetBitmap(ICON_NEW, icon_size);
	wxBitmap open_bitmap = IMAGE_MANAGER.GetBitmap(ICON_OPEN, icon_size);
	wxBitmap save_bitmap = IMAGE_MANAGER.GetBitmap(ICON_SAVE, icon_size);
	wxBitmap saveas_bitmap = IMAGE_MANAGER.GetBitmap(ICON_SAVE, icon_size);
	wxBitmap undo_bitmap = IMAGE_MANAGER.GetBitmap(ICON_UNDO, icon_size);
	wxBitmap redo_bitmap = IMAGE_MANAGER.GetBitmap(ICON_REDO, icon_size);
	wxBitmap cut_bitmap = IMAGE_MANAGER.GetBitmap(ICON_CUT, icon_size);
	wxBitmap copy_bitmap = IMAGE_MANAGER.GetBitmap(ICON_COPY, icon_size);
	wxBitmap paste_bitmap = IMAGE_MANAGER.GetBitmap(ICON_PASTE, icon_size);
	wxBitmap find_bitmap = IMAGE_MANAGER.GetBitmap(ICON_FIND, icon_size);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	toolbar->SetToolBitmapSize(icon_size);
	toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map (Ctrl+N) - Create a new empty map", "Create a new empty map", nullptr);
	toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map (Ctrl+O) - Open an existing map", "Open an existing map", nullptr);
	toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map (Ctrl+S)", "Save the current map", nullptr);
	toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As... (Ctrl+Alt+S)", "Save the current map with a new name", nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo (Ctrl+Z)", "Undo the last action", nullptr);
	toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo (Ctrl+Shift+Z)", "Redo the last undone action", nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut (Ctrl+X)", "Cut selection to clipboard", nullptr);
	toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy (Ctrl+C)", "Copy selection to clipboard", nullptr);
	toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste (Ctrl+V)", "Paste from clipboard", nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(MAIN_FRAME_MENU + MenuBar::JUMP_TO_BRUSH, wxEmptyString, find_bitmap, wxNullBitmap, wxITEM_NORMAL, "Jump to Brush (J)", "Find brush or item", nullptr);
	toolbar->Realize();

	toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &StandardToolBar::OnButtonClick, this);
}

StandardToolBar::~StandardToolBar() {
	toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &StandardToolBar::OnButtonClick, this);
}

void StandardToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		bool canUndo = editor->actionQueue->canUndo();
		toolbar->EnableTool(wxID_UNDO, canUndo);
		toolbar->SetToolShortHelp(wxID_UNDO, canUndo ? "Undo (Ctrl+Z)" : "Undo (Ctrl+Z) - Nothing to undo");

		bool canRedo = editor->actionQueue->canRedo();
		toolbar->EnableTool(wxID_REDO, canRedo);
		toolbar->SetToolShortHelp(wxID_REDO, canRedo ? "Redo (Ctrl+Shift+Z)" : "Redo (Ctrl+Shift+Z) - Nothing to redo");

		bool canPaste = editor->copybuffer.canPaste();
		toolbar->EnableTool(wxID_PASTE, canPaste);
		toolbar->SetToolShortHelp(wxID_PASTE, canPaste ? "Paste (Ctrl+V)" : "Paste (Ctrl+V) - Clipboard empty");
	} else {
		toolbar->EnableTool(wxID_UNDO, false);
		toolbar->SetToolShortHelp(wxID_UNDO, "Undo (Ctrl+Z) - No editor open");
		toolbar->EnableTool(wxID_REDO, false);
		toolbar->SetToolShortHelp(wxID_REDO, "Redo (Ctrl+Shift+Z) - No editor open");
		toolbar->EnableTool(wxID_PASTE, false);
		toolbar->SetToolShortHelp(wxID_PASTE, "Paste (Ctrl+V) - No editor open");
	}

	bool has_map = editor != nullptr;
	bool is_host = has_map && !editor->live_manager.IsClient();

	toolbar->EnableTool(wxID_SAVE, is_host);
	toolbar->SetToolShortHelp(wxID_SAVE, is_host ? "Save Map (Ctrl+S)" : (has_map ? "Save Map (Ctrl+S) - Client cannot save" : "Save Map (Ctrl+S) - No map open"));

	toolbar->EnableTool(wxID_SAVEAS, is_host);
	toolbar->SetToolShortHelp(wxID_SAVEAS, is_host ? "Save Map As... (Ctrl+Alt+S)" : (has_map ? "Save Map As... (Ctrl+Alt+S) - Client cannot save" : "Save Map As... (Ctrl+Alt+S) - No map open"));

	toolbar->EnableTool(wxID_CUT, has_map);
	toolbar->SetToolShortHelp(wxID_CUT, has_map ? "Cut (Ctrl+X)" : "Cut (Ctrl+X) - No map open");

	toolbar->EnableTool(wxID_COPY, has_map);
	toolbar->SetToolShortHelp(wxID_COPY, has_map ? "Copy (Ctrl+C)" : "Copy (Ctrl+C) - No map open");

	toolbar->Refresh();
}

void StandardToolBar::OnButtonClick(wxCommandEvent& event) {
	switch (event.GetId()) {
		case wxID_NEW:
			g_gui.NewMap();
			break;
		case wxID_OPEN:
			g_gui.OpenMap();
			break;
		case wxID_SAVE:
			g_gui.SaveMap();
			break;
		case wxID_SAVEAS:
			g_gui.SaveMapAs();
			break;
		case wxID_UNDO:
			g_gui.DoUndo();
			break;
		case wxID_REDO:
			g_gui.DoRedo();
			break;
		case wxID_CUT:
			g_gui.DoCut();
			break;
		case wxID_COPY:
			g_gui.DoCopy();
			break;
		case wxID_PASTE:
			g_gui.PreparePaste();
			break;
		default:
			event.Skip();
			break;
	}
}
