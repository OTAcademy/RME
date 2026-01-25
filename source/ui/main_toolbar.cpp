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

#include "ui/toolbar/toolbar_persistence.h"

MainToolBar::MainToolBar(wxWindow* parent, wxAuiManager* manager) {
	standard_toolbar_component = newd StandardToolBar(parent);

	brush_toolbar_component = newd BrushToolBar(parent);
	position_toolbar_component = newd PositionToolBar(parent);
	size_toolbar_component = newd SizeToolBar(parent);
	light_toolbar_component = newd LightToolBar(parent);

	manager->AddPane(standard_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(StandardToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	manager->AddPane(brush_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(BrushToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));
	manager->AddPane(size_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(SizeToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(3).Floatable(false));
	manager->AddPane(position_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(PositionToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(4).Floatable(false));
	manager->AddPane(light_toolbar_component->GetToolbar(), wxAuiPaneInfo().Name(LightToolBar::PANE_NAME).ToolbarPane().Top().Row(1).Position(5).Floatable(false));

	HideAll();
	UpdateButtons();
	UpdateBrushButtons();
}

MainToolBar::~MainToolBar() {
	delete standard_toolbar_component;
	delete brush_toolbar_component;
	delete position_toolbar_component;
	delete size_toolbar_component;
	delete light_toolbar_component;
}

void MainToolBar::UpdateButtons() {
	standard_toolbar_component->Update();

	brush_toolbar_component->Update();
	position_toolbar_component->Update();
	size_toolbar_component->Update();
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
	ToolbarPersistence::LoadPerspective(manager, this);
}

void MainToolBar::SavePerspective() {
	wxAuiManager* manager = g_gui.GetAuiManager();
	ToolbarPersistence::SavePerspective(manager, this);
}

wxAuiPaneInfo& MainToolBar::GetPane(ToolBarID id) {
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager) {
		return wxAuiNullPaneInfo;
	}

	switch (id) {
		case TOOLBAR_STANDARD:
			return manager->GetPane(StandardToolBar::PANE_NAME);
		case TOOLBAR_BRUSHES:
			return manager->GetPane(BrushToolBar::PANE_NAME);
		case TOOLBAR_POSITION:
			return manager->GetPane(PositionToolBar::PANE_NAME);
		case TOOLBAR_SIZES:
			return manager->GetPane(SizeToolBar::PANE_NAME);
		case TOOLBAR_LIGHT:
			return manager->GetPane(LightToolBar::PANE_NAME);
		default:
			return wxAuiNullPaneInfo;
	}
}
