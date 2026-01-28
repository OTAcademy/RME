#include "ui/menubar/palette_menu_handler.h"
#include "app/main.h"
#include "ui/gui.h"
#include "palette/house/house_palette.h"

PaletteMenuHandler::PaletteMenuHandler(MainFrame* frame, MainMenuBar* menubar) :
	frame(frame), menubar(menubar) {
}

PaletteMenuHandler::~PaletteMenuHandler() {
}

void PaletteMenuHandler::OnNewPalette(wxCommandEvent& event) {
	g_gui.NewPalette();
}

void PaletteMenuHandler::OnSelectTerrainPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_TERRAIN);
}

void PaletteMenuHandler::OnSelectDoodadPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_DOODAD);
}

void PaletteMenuHandler::OnSelectItemPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_ITEM);
}

void PaletteMenuHandler::OnSelectCollectionPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_COLLECTION);
}

void PaletteMenuHandler::OnSelectHousePalette(wxCommandEvent& WXUNUSED(event)) {
	if (g_gui.house_palette) {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_gui.house_palette);
		info.Show(!info.IsShown());
		g_gui.aui_manager->Update();
	}
}

void PaletteMenuHandler::OnSelectCreaturePalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_CREATURE);
}

void PaletteMenuHandler::OnSelectWaypointPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_WAYPOINT);
}

void PaletteMenuHandler::OnSelectRawPalette(wxCommandEvent& WXUNUSED(event)) {
	g_gui.SelectPalettePage(TILESET_RAW);
}
