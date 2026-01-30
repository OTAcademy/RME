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

#include <wx/wfstream.h>
#include <spdlog/spdlog.h>
#include <wx/display.h>
#include <format>

#include "ui/gui.h"
#include "util/file_system.h"
#include "ui/main_menubar.h"
#include "ui/dialog_util.h"

#include "editor/editor.h"
#include "editor/action_queue.h"
#include "editor/editor_factory.h"
#include "ui/managers/minimap_manager.h"
#include "brushes/managers/doodad_preview_manager.h"
#include "ui/managers/status_manager.h"
#include "brushes/brush.h"
#include "map/map.h"
#include "game/sprites.h"
#include "game/materials.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/spawn/spawn_brush.h"

#include "ui/controls/item_buttons.h"
#include "ui/result_window.h"
#include "rendering/ui/minimap_window.h"
#include "palette/palette_window.h"
#include "palette/house/house_palette.h"
#include "rendering/ui/map_display.h"
#include "app/application.h"
#include "ui/welcome_dialog.h"
#include "ui/tool_options_window.h"

#include "live/live_client.h"
#include "live/live_tab.h"
#include "live/live_server.h"

#ifdef __WXOSX__
	#include <AGL/agl.h>
#endif

const wxEventType EVT_UPDATE_MENUS = wxNewEventType();

// Global GUI instance
GUI g_gui;

// GUI class implementation
GUI::GUI() :
	aui_manager(nullptr),
	root(nullptr),
	secondary_map(nullptr),
	tool_options(nullptr),
	mode(SELECTION_MODE),
	pasting(false),
	disabled_counter(0),
	hotkeys_enabled(true) {
}

GUI::~GUI() {
	delete aui_manager;
}

// OpenGL context management moved to GLContextManager

void GUI::AddPendingCanvasEvent(wxEvent& event) {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		mapTab->GetCanvas()->GetEventHandler()->AddPendingEvent(event);
	}
}

void GUI::UpdateMenus() {
	wxCommandEvent evt(EVT_UPDATE_MENUS);
	g_gui.root->AddPendingEvent(evt);
}

void GUI::ShowToolbar(ToolBarID id, bool show) {
	if (root && root->GetAuiToolBar()) {
		root->GetAuiToolBar()->Show(id, show);
	}
}

void GUI::SwitchMode() {
	if (mode == DRAWING_MODE) {
		SetSelectionMode();
	} else {
		SetDrawingMode();
	}
}

void GUI::SetSelectionMode() {
	if (mode == SELECTION_MODE) {
		return;
	}

	if (GetCurrentBrush() && GetCurrentBrush()->isDoodad()) {
		secondary_map = nullptr;
	}

	tabbook->OnSwitchEditorMode(SELECTION_MODE);
	mode = SELECTION_MODE;
}

void GUI::SetDrawingMode() {
	if (mode == DRAWING_MODE) {
		return;
	}

	std::set<MapTab*> al;
	for (int idx = 0; idx < tabbook->GetTabCount(); ++idx) {
		EditorTab* editorTab = tabbook->GetTab(idx);
		if (auto* mapTab = dynamic_cast<MapTab*>(editorTab)) {
			if (al.contains(mapTab)) {
				continue;
			}

			Editor* editor = mapTab->GetEditor();
			editor->selection.start();
			editor->selection.clear();
			editor->selection.finish();
			al.insert(mapTab);
		}
	}

	if (GetCurrentBrush() && GetCurrentBrush()->isDoodad()) {
		secondary_map = g_doodad_preview.GetBufferMap();
	} else {
		secondary_map = nullptr;
	}

	tabbook->OnSwitchEditorMode(DRAWING_MODE);
	mode = DRAWING_MODE;
}

void GUI::RefreshView() {
	EditorTab* editorTab = GetCurrentTab();
	if (!editorTab) {
		return;
	}

	if (!dynamic_cast<MapTab*>(editorTab)) {
		editorTab->GetWindow()->Refresh();
		return;
	}

	std::vector<EditorTab*> editorTabs;
	for (int32_t index = 0; index < tabbook->GetTabCount(); ++index) {
		auto* mapTab = dynamic_cast<MapTab*>(tabbook->GetTab(index));
		if (mapTab) {
			editorTabs.push_back(mapTab);
		}
	}

	for (EditorTab* editorTab : editorTabs) {
		editorTab->GetWindow()->Refresh();
	}
}

// Welcome Dialog moved to WelcomeManager

void GUI::UpdateMenubar() {
	root->UpdateMenubar();
}

void GUI::SetScreenCenterPosition(Position position) {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		mapTab->SetScreenCenterPosition(position);
	}
}

void GUI::DoCut() {
	g_editors.DoCut();
}
void GUI::DoCopy() {
	g_editors.DoCopy();
}
void GUI::DoPaste() {
	g_editors.DoPaste();
}
void GUI::PreparePaste() {
	g_editors.PreparePaste();
}
void GUI::StartPasting() {
	g_editors.StartPasting();
}
void GUI::EndPasting() {
	g_editors.EndPasting();
}

bool GUI::CanUndo() {
	return g_editors.CanUndo();
}
bool GUI::CanRedo() {
	return g_editors.CanRedo();
}
bool GUI::DoUndo() {
	return g_editors.DoUndo();
}
bool GUI::DoRedo() {
	return g_editors.DoRedo();
}

int GUI::GetCurrentFloor() {
	MapTab* tab = GetCurrentMapTab();
	ASSERT(tab);
	return tab->GetCanvas()->GetFloor();
}

void GUI::ChangeFloor(int new_floor) {
	MapTab* tab = GetCurrentMapTab();
	if (tab) {
		int old_floor = GetCurrentFloor();
		if (new_floor < 0 || new_floor > MAP_MAX_LAYER) {
			return;
		}

		if (old_floor != new_floor) {
			tab->GetCanvas()->ChangeFloor(new_floor);
		}
	}
}

double GUI::GetCurrentZoom() {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		return mapTab->GetCanvas()->GetZoom();
	}
	return 1.0;
}

void GUI::SetCurrentZoom(double zoom) {
	MapTab* mapTab = GetCurrentMapTab();
	if (mapTab) {
		mapTab->GetCanvas()->SetZoom(zoom);
	}
}

// Search Results moved to SearchManager

void GUI::FitViewToMap() {
	for (int index = 0; index < tabbook->GetTabCount(); ++index) {
		if (auto* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index))) {
			tab->GetView()->FitToMap();
		}
	}
}

void GUI::FitViewToMap(MapTab* mt) {
	for (int index = 0; index < tabbook->GetTabCount(); ++index) {
		if (auto* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index))) {
			if (tab->HasSameReference(mt)) {
				tab->GetView()->FitToMap();
			}
		}
	}
}

void GUI::FillDoodadPreviewBuffer() {
	g_brush_manager.FillDoodadPreviewBuffer();
}
void GUI::SelectBrush() {
	g_brush_manager.SelectBrush();
}
bool GUI::SelectBrush(const Brush* brush, PaletteType pt) {
	return g_brush_manager.SelectBrush(brush, pt);
}
void GUI::SelectPreviousBrush() {
	g_brush_manager.SelectPreviousBrush();
}
void GUI::SelectBrushInternal(Brush* brush) {
	g_brush_manager.SelectBrushInternal(brush);
}
Brush* GUI::GetCurrentBrush() const {
	return g_brush_manager.GetCurrentBrush();
}
BrushShape GUI::GetBrushShape() const {
	return g_brush_manager.GetBrushShape();
}
int GUI::GetBrushSize() const {
	return g_brush_manager.GetBrushSize();
}
int GUI::GetBrushVariation() const {
	return g_brush_manager.GetBrushVariation();
}
int GUI::GetSpawnTime() const {
	return g_brush_manager.GetSpawnTime();
}
void GUI::SetSpawnTime(int time) {
	g_brush_manager.SetSpawnTime(time);
}
void GUI::SetLightIntensity(float v) {
	g_brush_manager.SetLightIntensity(v);
}
float GUI::GetLightIntensity() const {
	return g_brush_manager.GetLightIntensity();
}
void GUI::SetAmbientLightLevel(float v) {
	g_brush_manager.SetAmbientLightLevel(v);
}
float GUI::GetAmbientLightLevel() const {
	return g_brush_manager.GetAmbientLightLevel();
}
void GUI::SetBrushSize(int nz) {
	g_brush_manager.SetBrushSize(nz);
}
void GUI::SetBrushSizeInternal(int nz) {
	g_brush_manager.SetBrushSizeInternal(nz);
	if (tool_options) {
		tool_options->UpdateBrushSize(GetBrushShape(), nz);
	}
}
void GUI::SetBrushShape(BrushShape bs) {
	g_brush_manager.SetBrushShape(bs);
}
void GUI::SetBrushVariation(int nz) {
	g_brush_manager.SetBrushVariation(nz);
}
void GUI::SetBrushThickness(int low, int ceil) {
	g_brush_manager.SetBrushThickness(low, ceil);
}
void GUI::SetBrushThickness(bool on, int low, int ceil) {
	g_brush_manager.SetBrushThickness(on, low, ceil);
}
void GUI::DecreaseBrushSize(bool wrap) {
	g_brush_manager.DecreaseBrushSize(wrap);
}
void GUI::IncreaseBrushSize(bool wrap) {
	g_brush_manager.IncreaseBrushSize(wrap);
}
void GUI::SetDoorLocked(bool on) {
	g_brush_manager.SetDoorLocked(on);
}
bool GUI::HasDoorLocked() {
	return g_brush_manager.HasDoorLocked();
}

EditorTab* GUI::GetCurrentTab() {
	return g_editors.GetCurrentTab();
}
EditorTab* GUI::GetTab(int idx) {
	return g_editors.GetTab(idx);
}
int GUI::GetTabCount() const {
	return g_editors.GetTabCount();
}
bool GUI::IsAnyEditorOpen() const {
	return g_editors.IsAnyEditorOpen();
}
bool GUI::IsEditorOpen() const {
	return g_editors.IsEditorOpen();
}
void GUI::CloseCurrentEditor() {
	g_editors.CloseCurrentEditor();
}
Editor* GUI::GetCurrentEditor() {
	return g_editors.GetCurrentEditor();
}
MapTab* GUI::GetCurrentMapTab() const {
	return g_editors.GetCurrentMapTab();
}
void GUI::CycleTab(bool forward) {
	g_editors.CycleTab(forward);
}
bool GUI::CloseLiveEditors(LiveSocket* sock) {
	return g_editors.CloseLiveEditors(sock);
}
bool GUI::CloseAllEditors() {
	return g_editors.CloseAllEditors();
}
void GUI::NewMapView() {
	g_editors.NewMapView();
}

bool GUI::NewMap() {
	return g_editors.NewMap();
}
void GUI::OpenMap() {
	g_editors.OpenMap();
}
void GUI::SaveMap() {
	g_editors.SaveMap();
}
void GUI::SaveMapAs() {
	g_editors.SaveMapAs();
}
bool GUI::LoadMap(const FileName& fileName) {
	return g_editors.LoadMap(fileName);
}

Map& GUI::GetCurrentMap() {
	return g_editors.GetCurrentMap();
}
int GUI::GetOpenMapCount() {
	return g_editors.GetOpenMapCount();
}
bool GUI::ShouldSave() {
	return g_editors.ShouldSave();
}
void GUI::SaveCurrentMap(FileName filename, bool showdialog) {
	g_editors.SaveCurrentMap(filename, showdialog);
}
void GUI::SaveCurrentMap(bool showdialog) {
	g_editors.SaveCurrentMap(FileName(""), showdialog);
}

PaletteWindow* GUI::NewPalette() {
	return g_palettes.NewPalette();
}
void GUI::ActivatePalette(PaletteWindow* p) {
	g_palettes.ActivatePalette(p);
	if (p && tool_options) {
		tool_options->SetPaletteType(p->GetSelectedPage());
	}
}
void GUI::RebuildPalettes() {
	g_palettes.RebuildPalettes();
}
void GUI::RefreshPalettes(Map* m, bool usedfault) {
	g_palettes.RefreshPalettes(m, usedfault);
	if (house_palette) {
		house_palette->SetMap(m ? m : (usedfault ? (IsEditorOpen() ? &GetCurrentMap() : nullptr) : nullptr));
	}
}
void GUI::RefreshOtherPalettes(PaletteWindow* p) {
	g_palettes.RefreshOtherPalettes(p);
}
void GUI::ShowPalette() {
	g_palettes.ShowPalette();
}
void GUI::SelectPalettePage(PaletteType pt) {
	g_palettes.SelectPalettePage(pt);
}
PaletteWindow* GUI::GetPalette() {
	return g_palettes.GetPalette();
}
const std::list<PaletteWindow*>& GUI::GetPalettes() {
	return g_palettes.palettes;
}
void GUI::DestroyPalettes() {
	g_palettes.DestroyPalettes();
}
PaletteWindow* GUI::CreatePalette() {
	return g_palettes.CreatePalette();
}

void SetWindowToolTip(wxWindow* a, const wxString& tip) {
	a->SetToolTip(tip);
}

void SetWindowToolTip(wxWindow* a, wxWindow* b, const wxString& tip) {
	a->SetToolTip(tip);
	b->SetToolTip(tip);
}
