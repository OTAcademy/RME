//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/ui/map_menu_handler.h"
#include "rendering/ui/map_display.h"
#include "rendering/ui/clipboard_handler.h"
#include "rendering/ui/popup_action_handler.h"
#include "brush_utility.h" // For BrushSelector if needed, though map_display used it directly from "rendering/ui/brush_selector.h" which seems to be "brush_utility.h" or similar?
// Actually map_display.cpp included "rendering/ui/brush_selector.h".
// Let's check includes in map_display.cpp again.
// #include "rendering/ui/brush_selector.h"
// #include "rendering/ui/popup_action_handler.h"
// #include "rendering/ui/clipboard_handler.h"

#include "rendering/ui/brush_selector.h"
#include "map_popup_menu.h"
#include "editor.h"
#include "gui_ids.h"

MapMenuHandler::MapMenuHandler(MapCanvas* canvas, Editor& editor) :
	canvas(canvas),
	editor(editor) {
}

void MapMenuHandler::BindEvents() {
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCut, this, MAP_POPUP_MENU_CUT);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCopy, this, MAP_POPUP_MENU_COPY);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCopyPosition, this, MAP_POPUP_MENU_COPY_POSITION);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnPaste, this, MAP_POPUP_MENU_PASTE);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnDelete, this, MAP_POPUP_MENU_DELETE);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCopyServerId, this, MAP_POPUP_MENU_COPY_SERVER_ID);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCopyClientId, this, MAP_POPUP_MENU_COPY_CLIENT_ID);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnCopyName, this, MAP_POPUP_MENU_COPY_NAME);

	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnRotateItem, this, MAP_POPUP_MENU_ROTATE);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnGotoDestination, this, MAP_POPUP_MENU_GOTO);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSwitchDoor, this, MAP_POPUP_MENU_SWITCH_DOOR);

	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectRAWBrush, this, MAP_POPUP_MENU_SELECT_RAW_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectGroundBrush, this, MAP_POPUP_MENU_SELECT_GROUND_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectDoodadBrush, this, MAP_POPUP_MENU_SELECT_DOODAD_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectCollectionBrush, this, MAP_POPUP_MENU_SELECT_COLLECTION_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectDoorBrush, this, MAP_POPUP_MENU_SELECT_DOOR_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectWallBrush, this, MAP_POPUP_MENU_SELECT_WALL_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectCarpetBrush, this, MAP_POPUP_MENU_SELECT_CARPET_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectTableBrush, this, MAP_POPUP_MENU_SELECT_TABLE_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectCreatureBrush, this, MAP_POPUP_MENU_SELECT_CREATURE_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectSpawnBrush, this, MAP_POPUP_MENU_SELECT_SPAWN_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectHouseBrush, this, MAP_POPUP_MENU_SELECT_HOUSE_BRUSH);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnSelectMoveTo, this, MAP_POPUP_MENU_MOVE_TO_TILESET);

	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnProperties, this, MAP_POPUP_MENU_PROPERTIES);
	canvas->Bind(wxEVT_MENU, &MapMenuHandler::OnBrowseTile, this, MAP_POPUP_MENU_BROWSE_TILE);
}

void MapMenuHandler::OnCopy(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copy(editor, canvas->GetFloor());
}

void MapMenuHandler::OnCut(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::cut(editor, canvas->GetFloor());
}

void MapMenuHandler::OnPaste(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::paste();
}

void MapMenuHandler::OnDelete(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::doDelete(editor);
}

void MapMenuHandler::OnCopyPosition(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyPosition(editor.selection);
}

void MapMenuHandler::OnCopyServerId(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyServerId(editor.selection);
}

void MapMenuHandler::OnCopyClientId(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyClientId(editor.selection);
}

void MapMenuHandler::OnCopyName(wxCommandEvent& WXUNUSED(event)) {
	ClipboardHandler::copyName(editor.selection);
}

void MapMenuHandler::OnBrowseTile(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::BrowseTile(editor, canvas->cursor_x, canvas->cursor_y);
}

void MapMenuHandler::OnRotateItem(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::RotateItem(editor);
}

void MapMenuHandler::OnGotoDestination(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::GotoDestination(editor);
}

void MapMenuHandler::OnSwitchDoor(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::SwitchDoor(editor);
}

void MapMenuHandler::OnSelectRAWBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectRAWBrush(editor.selection);
}

void MapMenuHandler::OnSelectGroundBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectGroundBrush(editor.selection);
}

void MapMenuHandler::OnSelectDoodadBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectDoodadBrush(editor.selection);
}

void MapMenuHandler::OnSelectDoorBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectDoorBrush(editor.selection);
}

void MapMenuHandler::OnSelectWallBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectWallBrush(editor.selection);
}

void MapMenuHandler::OnSelectCarpetBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCarpetBrush(editor.selection);
}

void MapMenuHandler::OnSelectTableBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectTableBrush(editor.selection);
}

void MapMenuHandler::OnSelectHouseBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectHouseBrush(editor, editor.selection);
}

void MapMenuHandler::OnSelectCollectionBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCollectionBrush(editor.selection);
}

void MapMenuHandler::OnSelectCreatureBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectCreatureBrush(editor.selection);
}

void MapMenuHandler::OnSelectSpawnBrush(wxCommandEvent& WXUNUSED(event)) {
	BrushSelector::SelectSpawnBrush();
}

void MapMenuHandler::OnSelectMoveTo(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::SelectMoveTo(editor);
}

void MapMenuHandler::OnProperties(wxCommandEvent& WXUNUSED(event)) {
	PopupActionHandler::OpenProperties(editor);
}
