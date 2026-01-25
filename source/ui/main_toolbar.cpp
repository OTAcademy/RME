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
#include "ui/main_toolbar.h"
#include "ui/gui.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "app/settings.h"
#include "brushes/brush.h"
#include "brushes/managers/brush_manager.h"
#include "ui/artprovider.h"
#include <wx/artprov.h>
#include <wx/mstream.h>

const wxString MainToolBar::STANDARD_BAR_NAME = "standard_toolbar";
const wxString MainToolBar::LIGHT_BAR_NAME = "light_toolbar";

MainToolBar::MainToolBar(wxWindow* parent, wxAuiManager* manager) {
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

	standard_toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	standard_toolbar->SetToolBitmapSize(icon_size);
	standard_toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As...", wxEmptyString, nullptr);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo", wxEmptyString, nullptr);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy", wxEmptyString, nullptr);
	standard_toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste", wxEmptyString, nullptr);
	standard_toolbar->Realize();

	brush_toolbar_component = newd BrushToolBar(parent);
	position_toolbar_component = newd PositionToolBar(parent);
	size_toolbar_component = newd SizeToolBar(parent);

	manager->AddPane(standard_toolbar, wxAuiPaneInfo().Name(STANDARD_BAR_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	manager->AddPane(brush_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(BrushToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));
	manager->AddPane(position_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(PositionToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(4).Floatable(false));
	manager->AddPane(size_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(SizeToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(3).Floatable(false));

	light_toolbar = newd wxAuiToolBar(parent, TOOLBAR_LIGHT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORZ_TEXT);
	light_toolbar->SetToolBitmapSize(icon_size);

	wxStaticText* intensity_label = newd wxStaticText(light_toolbar, wxID_ANY, "Intensity:");
	light_slider = newd wxSlider(light_toolbar, ID_LIGHT_INTENSITY_SLIDER, 100, 0, 200, wxDefaultPosition, parent->FromDIP(wxSize(100, 20)));
	light_slider->SetToolTip("Global Light Intensity");

	wxStaticText* ambient_label = newd wxStaticText(light_toolbar, wxID_ANY, "Ambient:");
	ambient_slider = newd wxSlider(light_toolbar, ID_AMBIENT_LIGHT_SLIDER, 50, 0, 100, wxDefaultPosition, parent->FromDIP(wxSize(100, 20)));
	ambient_slider->SetToolTip("Ambient Light Level");

	light_toolbar->AddControl(intensity_label);
	light_toolbar->AddControl(light_slider);
	light_toolbar->AddSeparator();
	light_toolbar->AddControl(ambient_label);
	light_toolbar->AddControl(ambient_slider);

	light_toolbar->Realize();
	manager->AddPane(light_toolbar, wxAuiPaneInfo().Name(LIGHT_BAR_NAME).ToolbarPane().Top().Row(1).Position(5).Floatable(false));

	standard_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnStandardButtonClick, this);
	light_slider->Bind(wxEVT_SLIDER, &MainToolBar::OnLightSlider, this);
	ambient_slider->Bind(wxEVT_SLIDER, &MainToolBar::OnAmbientLightSlider, this);

	HideAll();
	UpdateButtons();
	UpdateBrushButtons();
}

MainToolBar::~MainToolBar() {
	standard_toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnStandardButtonClick, this);
	light_slider->Unbind(wxEVT_SLIDER, &MainToolBar::OnLightSlider, this);
	ambient_slider->Unbind(wxEVT_SLIDER, &MainToolBar::OnAmbientLightSlider, this);

	delete brush_toolbar_component;
	delete position_toolbar_component;
	delete size_toolbar_component;
}

void MainToolBar::UpdateButtons() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		standard_toolbar->EnableTool(wxID_UNDO, editor->actionQueue->canUndo());
		standard_toolbar->EnableTool(wxID_REDO, editor->actionQueue->canRedo());
		standard_toolbar->EnableTool(wxID_PASTE, editor->copybuffer.canPaste());
	} else {
		standard_toolbar->EnableTool(wxID_UNDO, false);
		standard_toolbar->EnableTool(wxID_REDO, false);
		standard_toolbar->EnableTool(wxID_PASTE, false);
	}

	bool has_map = editor != nullptr;
	bool is_host = has_map && !editor->live_manager.IsClient();

	standard_toolbar->EnableTool(wxID_SAVE, is_host);
	standard_toolbar->EnableTool(wxID_SAVEAS, is_host);
	standard_toolbar->EnableTool(wxID_CUT, has_map);
	standard_toolbar->EnableTool(wxID_COPY, has_map);

	brush_toolbar_component->Update();
	position_toolbar_component->Update();
	size_toolbar_component->Update();

	standard_toolbar->Refresh();
}

void MainToolBar::UpdateBrushButtons() {
	brush_toolbar_component->Update();
	g_gui.GetAuiManager()->Update();
}

void MainToolBar::UpdateBrushSize(BrushShape shape, int size) {
	size_toolbar_component->UpdateBrushSize(shape, size);
}

void MainToolBar::Show(ToolBarID id, bool show) {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (manager) {
		wxAuiPaneInfo& pane = GetPane(id);
		if (pane.IsOk()) {
			pane.Show(show);
			manager->Update();
		}
	}
}

void MainToolBar::HideAll(bool update) {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager) {
		return;
	}

	wxAuiPaneInfoArray& panes = manager->GetAllPanes();
	for (int i = 0, count = panes.GetCount(); i < count; ++i) {
		if (!panes.Item(i).IsToolbar()) {
			panes.Item(i).Hide();
		}
	}

	if (update) {
		manager->Update();
	}
}

void MainToolBar::LoadPerspective() {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager) {
		return;
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		std::string info = g_settings.getString(Config::TOOLBAR_STANDARD_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_STANDARD));
		}
		GetPane(TOOLBAR_STANDARD).Show();
	} else {
		GetPane(TOOLBAR_STANDARD).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_BRUSHES_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_BRUSHES));
		}
		GetPane(TOOLBAR_BRUSHES).Show();
	} else {
		GetPane(TOOLBAR_BRUSHES).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		std::string info = g_settings.getString(Config::TOOLBAR_POSITION_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_POSITION));
		}
		GetPane(TOOLBAR_POSITION).Show();
	} else {
		GetPane(TOOLBAR_POSITION).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_SIZES_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_SIZES));
		}
		GetPane(TOOLBAR_SIZES).Show();
	} else {
		GetPane(TOOLBAR_SIZES).Hide();
	}

	manager->Update();
}

void MainToolBar::SavePerspective() {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager) {
		return;
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_STANDARD));
		g_settings.setString(Config::TOOLBAR_STANDARD_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_BRUSHES));
		g_settings.setString(Config::TOOLBAR_BRUSHES_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_POSITION));
		g_settings.setString(Config::TOOLBAR_POSITION_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_SIZES));
		g_settings.setString(Config::TOOLBAR_SIZES_LAYOUT, info.ToStdString());
	}
}

void MainToolBar::OnStandardButtonClick(wxCommandEvent& event) {
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

wxAuiPaneInfo& MainToolBar::GetPane(ToolBarID id) {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager) {
		return wxAuiNullPaneInfo;
	}

	switch (id) {
		case TOOLBAR_STANDARD:
			return manager->GetPane(STANDARD_BAR_NAME);
		case TOOLBAR_BRUSHES:
			return manager->GetPane(BrushToolBar::PANE_NAME);
		case TOOLBAR_POSITION:
			return manager->GetPane(PositionToolBar::PANE_NAME);
		case TOOLBAR_SIZES:
			return manager->GetPane(SizeToolBar::PANE_NAME);
		case TOOLBAR_LIGHT:
			return manager->GetPane(LIGHT_BAR_NAME);
		default:
			return wxAuiNullPaneInfo;
	}
}

void MainToolBar::OnLightSlider(wxCommandEvent& event) {
	g_gui.SetLightIntensity(event.GetInt() / 100.0f);
	g_gui.RefreshView();
}

void MainToolBar::OnAmbientLightSlider(wxCommandEvent& event) {
	g_gui.SetAmbientLightLevel(event.GetInt() / 100.0f);
	g_gui.RefreshView();
}
