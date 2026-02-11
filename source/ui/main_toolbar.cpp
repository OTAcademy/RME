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
#include <wx/mstream.h>

#include "ui/toolbar/toolbar_persistence.h"

#include "ui/toolbar/toolbar_factory.h"
#include "ui/toolbar/toolbar_layout.h"

MainToolBar::MainToolBar(wxWindow* parent, wxAuiManager* manager) {
	registry = ToolbarFactory::CreateToolbars(parent);
	ToolbarLayout::Configure(manager, registry.get());

	HideAll();
	UpdateButtons();
	UpdateBrushButtons();
}

MainToolBar::~MainToolBar() {
	// Unique ptr handles deletion
}

void MainToolBar::UpdateButtons() {
	registry->UpdateAll();
}

void MainToolBar::UpdateBrushButtons() {
	if (auto tb = registry->GetBrushToolbar()) {
		tb->Update();
	}
	g_gui.GetAuiManager()->Update();
}

void MainToolBar::UpdateBrushSize(BrushShape shape, int size) {
	if (auto tb = registry->GetSizeToolbar()) {
		tb->UpdateBrushSize(shape, size);
	}
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
	return registry->GetPane(id, g_gui.GetAuiManager());
}
