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
#include "util/image_manager.h"

const wxString BrushToolBar::PANE_NAME = "brush_toolbar";

BrushToolBar::BrushToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));

	wxBitmap border_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_OPTIONAL_BORDER_SMALL, icon_size);
	wxBitmap eraser_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_ERASER_SMALL, icon_size);
	wxBitmap pz_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_PROTECTION_ZONE_SMALL, icon_size);
	wxBitmap nopvp_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_NO_PVP_ZONE_SMALL, icon_size);
	wxBitmap nologout_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_NO_LOGOUT_ZONE_SMALL, icon_size);
	wxBitmap pvp_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_PVP_ZONE_SMALL, icon_size);
	wxBitmap normal_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_NORMAL_SMALL, icon_size);
	wxBitmap locked_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_LOCKED_SMALL, icon_size);
	wxBitmap magic_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_MAGIC_SMALL, icon_size);
	wxBitmap quest_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_QUEST_SMALL, icon_size);
	wxBitmap normal_alt_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_NORMAL_ALT_SMALL, icon_size);
	wxBitmap archway_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_DOOR_ARCHWAY_SMALL, icon_size);

	wxBitmap hatch_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_WINDOW_HATCH_SMALL, icon_size);
	wxBitmap window_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_WINDOW_NORMAL_SMALL, icon_size);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_BRUSHES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	toolbar->SetToolBitmapSize(icon_size);
	toolbar->AddTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, wxEmptyString, border_bitmap, wxNullBitmap, wxITEM_CHECK, "Border (Add borders to ground)", "Add automatic borders to ground tiles", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_ERASER, wxEmptyString, eraser_bitmap, wxNullBitmap, wxITEM_CHECK, "Eraser (Clear tile content)", "Clear content from tiles", nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(PALETTE_TERRAIN_PZ_TOOL, wxEmptyString, pz_bitmap, wxNullBitmap, wxITEM_CHECK, "Protected Zone (Non-combat area)", "Mark area as Protected Zone (Non-combat)", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NOPVP_TOOL, wxEmptyString, nopvp_bitmap, wxNullBitmap, wxITEM_CHECK, "No PvP Zone (Non-PvP area)", "Mark area as No PvP Zone", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, wxEmptyString, nologout_bitmap, wxNullBitmap, wxITEM_CHECK, "No Logout Zone (Prevents logout)", "Mark area as No Logout Zone", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_PVPZONE_TOOL, wxEmptyString, pvp_bitmap, wxNullBitmap, wxITEM_CHECK, "PvP Zone (Combat area)", "Mark area as PvP Zone", nullptr);
	toolbar->AddSeparator();

	toolbar->AddTool(PALETTE_TERRAIN_NORMAL_DOOR, wxEmptyString, normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door", "Place normal door", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_LOCKED_DOOR, wxEmptyString, locked_bitmap, wxNullBitmap, wxITEM_CHECK, "Locked Door", "Place locked door", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_MAGIC_DOOR, wxEmptyString, magic_bitmap, wxNullBitmap, wxITEM_CHECK, "Magic Door", "Place magic door (Level dependent)", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_QUEST_DOOR, wxEmptyString, quest_bitmap, wxNullBitmap, wxITEM_CHECK, "Quest Door", "Place quest door", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, wxEmptyString, normal_alt_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door (alt)", "Place alternative normal door", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_ARCHWAY_DOOR, wxEmptyString, archway_bitmap, wxNullBitmap, wxITEM_CHECK, "Archway", "Place archway", nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(PALETTE_TERRAIN_HATCH_DOOR, wxEmptyString, hatch_bitmap, wxNullBitmap, wxITEM_CHECK, "Hatch Window", "Place hatch window", nullptr);
	toolbar->AddTool(PALETTE_TERRAIN_WINDOW_DOOR, wxEmptyString, window_bitmap, wxNullBitmap, wxITEM_CHECK, "Window", "Place window", nullptr);
	toolbar->Realize();

	toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &BrushToolBar::OnToolbarClick, this);
}

BrushToolBar::~BrushToolBar() {
	toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &BrushToolBar::OnToolbarClick, this);
}

void BrushToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	bool has_map = editor != nullptr;

	auto updateTool = [&](int id, const wxString& name) {
		toolbar->EnableTool(id, has_map);
		toolbar->SetToolShortHelp(id, has_map ? name : name + " - No map open");
	};

	updateTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, "Border (Add borders to ground)");
	updateTool(PALETTE_TERRAIN_ERASER, "Eraser (Clear tile content)");
	updateTool(PALETTE_TERRAIN_PZ_TOOL, "Protected Zone (Non-combat area)");
	updateTool(PALETTE_TERRAIN_NOPVP_TOOL, "No PvP Zone (Non-PvP area)");
	updateTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, "No Logout Zone (Prevents logout)");
	updateTool(PALETTE_TERRAIN_PVPZONE_TOOL, "PvP Zone (Combat area)");
	updateTool(PALETTE_TERRAIN_NORMAL_DOOR, "Normal Door");
	updateTool(PALETTE_TERRAIN_LOCKED_DOOR, "Locked Door");
	updateTool(PALETTE_TERRAIN_MAGIC_DOOR, "Magic Door");
	updateTool(PALETTE_TERRAIN_QUEST_DOOR, "Quest Door");
	updateTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, "Normal Door (alt)");
	updateTool(PALETTE_TERRAIN_ARCHWAY_DOOR, "Archway");
	updateTool(PALETTE_TERRAIN_HATCH_DOOR, "Hatch Window");
	updateTool(PALETTE_TERRAIN_WINDOW_DOOR, "Window");

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
