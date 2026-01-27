//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/brush_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "editor/editor.h"
#include "brushes/managers/brush_manager.h"
#include "brushes/flag/flag_brush.h"
#include "brushes/door/door_brush.h"
#include "brushes/border/optional_border_brush.h"
#include "ui/pngfiles.h"
#include "ui/artprovider.h"
#include <wx/artprov.h>
#include <wx/mstream.h>

const wxString BrushToolBar::PANE_NAME = "brushes_toolbar";

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap _wxGetBitmapFromMemory(const unsigned char* data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) {
		return wxBitmap();
	}
	return wxBitmap(img, -1);
}

BrushToolBar::BrushToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));

	wxBitmap border_bitmap = loadPNGFile(optional_border_small_png);
	wxBitmap eraser_bitmap = loadPNGFile(eraser_small_png);
	wxBitmap pz_bitmap = wxArtProvider::GetBitmap(ART_PZ_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nopvp_bitmap = wxArtProvider::GetBitmap(ART_NOPVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nologout_bitmap = wxArtProvider::GetBitmap(ART_NOLOOUT_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap pvp_bitmap = wxArtProvider::GetBitmap(ART_PVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap normal_bitmap = wxArtProvider::GetBitmap(ART_DOOR_NORMAL_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap locked_bitmap = wxArtProvider::GetBitmap(ART_DOOR_LOCKED_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap magic_bitmap = wxArtProvider::GetBitmap(ART_DOOR_MAGIC_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap quest_bitmap = wxArtProvider::GetBitmap(ART_DOOR_QUEST_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap normal_alt_bitmap = wxArtProvider::GetBitmap(ART_DOOR_NORMAL_ALT_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap archway_bitmap = wxArtProvider::GetBitmap(ART_DOOR_ARCHWAY_SMALL, wxART_TOOLBAR, icon_size);

	wxBitmap hatch_bitmap = loadPNGFile(window_hatch_small_png);
	wxBitmap window_bitmap = loadPNGFile(window_normal_small_png);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_BRUSHES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	toolbar->SetToolBitmapSize(icon_size);
	toolbar->AddTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, wxEmptyString, border_bitmap, wxNullBitmap, wxITEM_CHECK, "Border", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_ERASER, wxEmptyString, eraser_bitmap, wxNullBitmap, wxITEM_CHECK, "Eraser", wxEmptyString, nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(PALETTE_TERRAIN_PZ_TOOL, wxEmptyString, pz_bitmap, wxNullBitmap, wxITEM_CHECK, "Protected Zone", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NOPVP_TOOL, wxEmptyString, nopvp_bitmap, wxNullBitmap, wxITEM_CHECK, "No PvP Zone", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, wxEmptyString, nologout_bitmap, wxNullBitmap, wxITEM_CHECK, "No Logout Zone", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_PVPZONE_TOOL, wxEmptyString, pvp_bitmap, wxNullBitmap, wxITEM_CHECK, "PvP Zone", wxEmptyString, nullptr);
	toolbar->AddSeparator();

	toolbar->AddTool(PALETTE_TERRAIN_NORMAL_DOOR, wxEmptyString, normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_LOCKED_DOOR, wxEmptyString, locked_bitmap, wxNullBitmap, wxITEM_CHECK, "Locked Door", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_MAGIC_DOOR, wxEmptyString, magic_bitmap, wxNullBitmap, wxITEM_CHECK, "Magic Door", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_QUEST_DOOR, wxEmptyString, quest_bitmap, wxNullBitmap, wxITEM_CHECK, "Quest Door", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, wxEmptyString, normal_alt_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door (alt)", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_ARCHWAY_DOOR, wxEmptyString, archway_bitmap, wxNullBitmap, wxITEM_CHECK, "Archway", wxEmptyString, nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(PALETTE_TERRAIN_HATCH_DOOR, wxEmptyString, hatch_bitmap, wxNullBitmap, wxITEM_CHECK, "Hatch Window", wxEmptyString, nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_WINDOW_DOOR, wxEmptyString, window_bitmap, wxNullBitmap, wxITEM_CHECK, "Window", wxEmptyString, nullptr);
	toolbar->Realize();

	toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &BrushToolBar::OnToolbarClick, this);
}

BrushToolBar::~BrushToolBar() {
	toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &BrushToolBar::OnToolbarClick, this);
}

void BrushToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	bool has_map = editor != nullptr;

	toolbar->EnableTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_ERASER, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_PZ_TOOL, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_NOPVP_TOOL, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_PVPZONE_TOOL, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_NORMAL_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_LOCKED_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_MAGIC_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_QUEST_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_ARCHWAY_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_HATCH_DOOR, has_map);
	toolbar->EnableTool(PALETTE_TERRAIN_WINDOW_DOOR, has_map);

	Brush* brush = g_gui.GetCurrentBrush();
	if (brush) {
		toolbar->ToggleTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, brush == g_brush_manager.optional_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_ERASER, brush == g_brush_manager.eraser);
		toolbar->ToggleTool(PALETTE_TERRAIN_PZ_TOOL, brush == g_brush_manager.pz_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_NOPVP_TOOL, brush == g_brush_manager.rook_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, brush == g_brush_manager.nolog_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_PVPZONE_TOOL, brush == g_brush_manager.pvp_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_DOOR, brush == g_brush_manager.normal_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_LOCKED_DOOR, brush == g_brush_manager.locked_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_MAGIC_DOOR, brush == g_brush_manager.magic_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_QUEST_DOOR, brush == g_brush_manager.quest_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, brush == g_brush_manager.normal_door_alt_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_ARCHWAY_DOOR, brush == g_brush_manager.archway_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_HATCH_DOOR, brush == g_brush_manager.hatch_door_brush);
		toolbar->ToggleTool(PALETTE_TERRAIN_WINDOW_DOOR, brush == g_brush_manager.window_door_brush);
	} else {
		// Untoggle all
		toolbar->ToggleTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_ERASER, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_PZ_TOOL, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_NOPVP_TOOL, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_PVPZONE_TOOL, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_LOCKED_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_MAGIC_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_QUEST_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_ARCHWAY_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_HATCH_DOOR, false);
		toolbar->ToggleTool(PALETTE_TERRAIN_WINDOW_DOOR, false);
	}

	toolbar->Refresh();
}

void BrushToolBar::OnToolbarClick(wxCommandEvent& event) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	switch (event.GetId()) {
		case PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL:
			g_gui.SelectBrush(g_brush_manager.optional_brush);
			break;
		case PALETTE_TERRAIN_ERASER:
			g_gui.SelectBrush(g_brush_manager.eraser);
			break;
		case PALETTE_TERRAIN_PZ_TOOL:
			g_gui.SelectBrush(g_brush_manager.pz_brush);
			break;
		case PALETTE_TERRAIN_NOPVP_TOOL:
			g_gui.SelectBrush(g_brush_manager.rook_brush);
			break;
		case PALETTE_TERRAIN_NOLOGOUT_TOOL:
			g_gui.SelectBrush(g_brush_manager.nolog_brush);
			break;
		case PALETTE_TERRAIN_PVPZONE_TOOL:
			g_gui.SelectBrush(g_brush_manager.pvp_brush);
			break;
		case PALETTE_TERRAIN_NORMAL_DOOR:
			g_gui.SelectBrush(g_brush_manager.normal_door_brush);
			break;
		case PALETTE_TERRAIN_LOCKED_DOOR:
			g_gui.SelectBrush(g_brush_manager.locked_door_brush);
			break;
		case PALETTE_TERRAIN_MAGIC_DOOR:
			g_gui.SelectBrush(g_brush_manager.magic_door_brush);
			break;
		case PALETTE_TERRAIN_QUEST_DOOR:
			g_gui.SelectBrush(g_brush_manager.quest_door_brush);
			break;
		case PALETTE_TERRAIN_NORMAL_ALT_DOOR:
			g_gui.SelectBrush(g_brush_manager.normal_door_alt_brush);
			break;
		case PALETTE_TERRAIN_ARCHWAY_DOOR:
			g_gui.SelectBrush(g_brush_manager.archway_door_brush);
			break;
		case PALETTE_TERRAIN_HATCH_DOOR:
			g_gui.SelectBrush(g_brush_manager.hatch_door_brush);
			break;
		case PALETTE_TERRAIN_WINDOW_DOOR:
			g_gui.SelectBrush(g_brush_manager.window_door_brush);
			break;
		default:
			break;
	}
}
