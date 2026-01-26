//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/standard_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "ui/artprovider.h"
#include <wx/artprov.h>

const wxString StandardToolBar::PANE_NAME = "standard_toolbar";

StandardToolBar::StandardToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	wxBitmap new_bitmap = wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR, icon_size);
	wxBitmap open_bitmap = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, icon_size);
	wxBitmap save_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, icon_size);
	wxBitmap saveas_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR, icon_size);
	wxBitmap undo_bitmap = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR, icon_size);
	wxBitmap redo_bitmap = wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR, icon_size);
	wxBitmap cut_bitmap = wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR, icon_size);
	wxBitmap copy_bitmap = wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR, icon_size);
	wxBitmap paste_bitmap = wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR, icon_size);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	toolbar->SetToolBitmapSize(icon_size);
	toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map (Ctrl+N)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map (Ctrl+O)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map (Ctrl+S)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As... (Ctrl+Alt+S)", wxEmptyString, nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo (Ctrl+Z)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo (Ctrl+Shift+Z)", wxEmptyString, nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut (Ctrl+X)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy (Ctrl+C)", wxEmptyString, nullptr);
	toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste (Ctrl+V)", wxEmptyString, nullptr);
	toolbar->Realize();

	toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &StandardToolBar::OnButtonClick, this);
}

StandardToolBar::~StandardToolBar() {
	toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &StandardToolBar::OnButtonClick, this);
}

void StandardToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		toolbar->EnableTool(wxID_UNDO, editor->actionQueue->canUndo());
		toolbar->EnableTool(wxID_REDO, editor->actionQueue->canRedo());
		toolbar->EnableTool(wxID_PASTE, editor->copybuffer.canPaste());
	} else {
		toolbar->EnableTool(wxID_UNDO, false);
		toolbar->EnableTool(wxID_REDO, false);
		toolbar->EnableTool(wxID_PASTE, false);
	}

	bool has_map = editor != nullptr;
	bool is_host = has_map && !editor->live_manager.IsClient();

	toolbar->EnableTool(wxID_SAVE, is_host);
	toolbar->EnableTool(wxID_SAVEAS, is_host);
	toolbar->EnableTool(wxID_CUT, has_map);
	toolbar->EnableTool(wxID_COPY, has_map);

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
			break;
	}
}
