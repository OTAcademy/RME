#include "ui/menubar/view_settings_handler.h"
#include "ui/main_menubar.h"
#include "ui/gui.h"
#include "app/preferences.h"
#include <format>

ViewSettingsHandler::ViewSettingsHandler(MainMenuBar* menuBar) :
	menuBar(menuBar) {
}

void ViewSettingsHandler::LoadValues() {
	using namespace MenuBar;

	menuBar->CheckItem(VIEW_TOOLBARS_BRUSHES, g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES));
	menuBar->CheckItem(VIEW_TOOLBARS_POSITION, g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION));
	menuBar->CheckItem(VIEW_TOOLBARS_SIZES, g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES));
	menuBar->CheckItem(VIEW_TOOLBARS_STANDARD, g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD));

	menuBar->CheckItem(SELECT_MODE_COMPENSATE, g_settings.getBoolean(Config::COMPENSATED_SELECT));

	if (menuBar->IsItemChecked(MenuBar::SELECT_MODE_CURRENT)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	} else if (menuBar->IsItemChecked(MenuBar::SELECT_MODE_LOWER)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	} else if (menuBar->IsItemChecked(MenuBar::SELECT_MODE_VISIBLE)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);
	}

	switch (g_settings.getInteger(Config::SELECTION_TYPE)) {
		case SELECT_CURRENT_FLOOR:
			menuBar->CheckItem(SELECT_MODE_CURRENT, true);
			break;
		case SELECT_ALL_FLOORS:
			menuBar->CheckItem(SELECT_MODE_LOWER, true);
			break;
		default:
		case SELECT_VISIBLE_FLOORS:
			menuBar->CheckItem(SELECT_MODE_VISIBLE, true);
			break;
	}

	menuBar->CheckItem(AUTOMAGIC, g_settings.getBoolean(Config::USE_AUTOMAGIC));

	menuBar->CheckItem(SHOW_SHADE, g_settings.getBoolean(Config::SHOW_SHADE));
	menuBar->CheckItem(SHOW_INGAME_BOX, g_settings.getBoolean(Config::SHOW_INGAME_BOX));
	menuBar->CheckItem(SHOW_LIGHTS, g_settings.getBoolean(Config::SHOW_LIGHTS));
	menuBar->CheckItem(SHOW_LIGHT_STR, g_settings.getBoolean(Config::SHOW_LIGHT_STR));
	menuBar->CheckItem(SHOW_TECHNICAL_ITEMS, g_settings.getBoolean(Config::SHOW_TECHNICAL_ITEMS));
	menuBar->CheckItem(SHOW_WAYPOINTS, g_settings.getBoolean(Config::SHOW_WAYPOINTS));
	menuBar->CheckItem(SHOW_ALL_FLOORS, g_settings.getBoolean(Config::SHOW_ALL_FLOORS));
	menuBar->CheckItem(GHOST_ITEMS, g_settings.getBoolean(Config::TRANSPARENT_ITEMS));
	menuBar->CheckItem(GHOST_HIGHER_FLOORS, g_settings.getBoolean(Config::TRANSPARENT_FLOORS));
	menuBar->CheckItem(SHOW_EXTRA, !g_settings.getBoolean(Config::SHOW_EXTRA));
	menuBar->CheckItem(SHOW_GRID, g_settings.getBoolean(Config::SHOW_GRID));
	menuBar->CheckItem(HIGHLIGHT_ITEMS, g_settings.getBoolean(Config::HIGHLIGHT_ITEMS));
	menuBar->CheckItem(HIGHLIGHT_LOCKED_DOORS, g_settings.getBoolean(Config::HIGHLIGHT_LOCKED_DOORS));
	menuBar->CheckItem(SHOW_CREATURES, g_settings.getBoolean(Config::SHOW_CREATURES));
	menuBar->CheckItem(SHOW_SPAWNS, g_settings.getBoolean(Config::SHOW_SPAWNS));
	menuBar->CheckItem(SHOW_SPECIAL, g_settings.getBoolean(Config::SHOW_SPECIAL_TILES));
	menuBar->CheckItem(SHOW_AS_MINIMAP, g_settings.getBoolean(Config::SHOW_AS_MINIMAP));
	menuBar->CheckItem(SHOW_ONLY_COLORS, g_settings.getBoolean(Config::SHOW_ONLY_TILEFLAGS));
	menuBar->CheckItem(SHOW_ONLY_MODIFIED, g_settings.getBoolean(Config::SHOW_ONLY_MODIFIED_TILES));
	menuBar->CheckItem(SHOW_HOUSES, g_settings.getBoolean(Config::SHOW_HOUSES));
	menuBar->CheckItem(SHOW_PATHING, g_settings.getBoolean(Config::SHOW_BLOCKING));
	menuBar->CheckItem(SHOW_TOOLTIPS, g_settings.getBoolean(Config::SHOW_TOOLTIPS));
	menuBar->CheckItem(SHOW_PREVIEW, g_settings.getBoolean(Config::SHOW_PREVIEW));
	menuBar->CheckItem(SHOW_WALL_HOOKS, g_settings.getBoolean(Config::SHOW_WALL_HOOKS));
	menuBar->CheckItem(SHOW_TOWNS, g_settings.getBoolean(Config::SHOW_TOWNS));
	menuBar->CheckItem(ALWAYS_SHOW_ZONES, g_settings.getBoolean(Config::ALWAYS_SHOW_ZONES));
	menuBar->CheckItem(EXT_HOUSE_SHADER, g_settings.getBoolean(Config::EXT_HOUSE_SHADER));

	menuBar->CheckItem(EXPERIMENTAL_FOG, g_settings.getBoolean(Config::EXPERIMENTAL_FOG));
}

void ViewSettingsHandler::OnChangeViewSettings(wxCommandEvent& event) {
	using namespace MenuBar;

	bool old_grid = g_settings.getBoolean(Config::SHOW_GRID);
	bool old_ghost = g_settings.getBoolean(Config::TRANSPARENT_ITEMS);

	g_settings.setInteger(Config::SHOW_ALL_FLOORS, menuBar->IsItemChecked(SHOW_ALL_FLOORS));
	if (menuBar->IsItemChecked(SHOW_ALL_FLOORS)) {
		menuBar->EnableItem(SELECT_MODE_VISIBLE, true);
		menuBar->EnableItem(SELECT_MODE_LOWER, true);
	} else {
		menuBar->EnableItem(SELECT_MODE_VISIBLE, false);
		menuBar->EnableItem(SELECT_MODE_LOWER, false);
		menuBar->CheckItem(SELECT_MODE_CURRENT, true);
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	}
	g_settings.setInteger(Config::TRANSPARENT_FLOORS, menuBar->IsItemChecked(GHOST_HIGHER_FLOORS));
	g_settings.setInteger(Config::TRANSPARENT_ITEMS, menuBar->IsItemChecked(GHOST_ITEMS));
	g_settings.setInteger(Config::SHOW_INGAME_BOX, menuBar->IsItemChecked(SHOW_INGAME_BOX));
	g_settings.setInteger(Config::SHOW_LIGHTS, menuBar->IsItemChecked(SHOW_LIGHTS));
	g_settings.setInteger(Config::SHOW_LIGHT_STR, menuBar->IsItemChecked(SHOW_LIGHT_STR));
	g_settings.setInteger(Config::SHOW_TECHNICAL_ITEMS, menuBar->IsItemChecked(SHOW_TECHNICAL_ITEMS));
	g_settings.setInteger(Config::SHOW_WAYPOINTS, menuBar->IsItemChecked(SHOW_WAYPOINTS));
	g_settings.setInteger(Config::SHOW_GRID, menuBar->IsItemChecked(SHOW_GRID));
	g_settings.setInteger(Config::SHOW_EXTRA, !menuBar->IsItemChecked(SHOW_EXTRA));

	g_settings.setInteger(Config::SHOW_SHADE, menuBar->IsItemChecked(SHOW_SHADE));
	g_settings.setInteger(Config::SHOW_SPECIAL_TILES, menuBar->IsItemChecked(SHOW_SPECIAL));
	g_settings.setInteger(Config::SHOW_AS_MINIMAP, menuBar->IsItemChecked(SHOW_AS_MINIMAP));
	g_settings.setInteger(Config::SHOW_ONLY_TILEFLAGS, menuBar->IsItemChecked(SHOW_ONLY_COLORS));
	g_settings.setInteger(Config::SHOW_ONLY_MODIFIED_TILES, menuBar->IsItemChecked(SHOW_ONLY_MODIFIED));
	g_settings.setInteger(Config::SHOW_CREATURES, menuBar->IsItemChecked(SHOW_CREATURES));
	g_settings.setInteger(Config::SHOW_SPAWNS, menuBar->IsItemChecked(SHOW_SPAWNS));
	g_settings.setInteger(Config::SHOW_HOUSES, menuBar->IsItemChecked(SHOW_HOUSES));
	g_settings.setInteger(Config::HIGHLIGHT_ITEMS, menuBar->IsItemChecked(HIGHLIGHT_ITEMS));
	g_settings.setInteger(Config::HIGHLIGHT_LOCKED_DOORS, menuBar->IsItemChecked(HIGHLIGHT_LOCKED_DOORS));
	g_settings.setInteger(Config::SHOW_BLOCKING, menuBar->IsItemChecked(SHOW_PATHING));
	g_settings.setInteger(Config::SHOW_TOOLTIPS, menuBar->IsItemChecked(SHOW_TOOLTIPS));
	g_settings.setInteger(Config::SHOW_PREVIEW, menuBar->IsItemChecked(SHOW_PREVIEW));
	g_settings.setInteger(Config::SHOW_WALL_HOOKS, menuBar->IsItemChecked(SHOW_WALL_HOOKS));
	g_settings.setInteger(Config::SHOW_TOWNS, menuBar->IsItemChecked(SHOW_TOWNS));
	g_settings.setInteger(Config::ALWAYS_SHOW_ZONES, menuBar->IsItemChecked(ALWAYS_SHOW_ZONES));
	g_settings.setInteger(Config::EXT_HOUSE_SHADER, menuBar->IsItemChecked(EXT_HOUSE_SHADER));

	g_settings.setInteger(Config::EXPERIMENTAL_FOG, menuBar->IsItemChecked(EXPERIMENTAL_FOG));

	bool new_grid = g_settings.getBoolean(Config::SHOW_GRID);
	if (old_grid != new_grid) {
		g_gui.SetStatusText(std::format("Grid: {}", new_grid ? "On" : "Off"));
	}

	bool new_ghost = g_settings.getBoolean(Config::TRANSPARENT_ITEMS);
	if (old_ghost != new_ghost) {
		g_gui.SetStatusText(std::format("Ghost Mode: {}", new_ghost ? "On" : "Off"));
	}

	g_gui.RefreshView();
}

void ViewSettingsHandler::OnToolbars(wxCommandEvent& event) {
	using namespace MenuBar;

	ActionID id = static_cast<ActionID>(event.GetId() - (wxID_HIGHEST + 1));
	switch (id) {
		case VIEW_TOOLBARS_BRUSHES:
			g_gui.ShowToolbar(TOOLBAR_BRUSHES, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_BRUSHES, event.IsChecked());
			break;
		case VIEW_TOOLBARS_POSITION:
			g_gui.ShowToolbar(TOOLBAR_POSITION, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_POSITION, event.IsChecked());
			break;
		case VIEW_TOOLBARS_SIZES:
			g_gui.ShowToolbar(TOOLBAR_SIZES, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_SIZES, event.IsChecked());
			break;
		case VIEW_TOOLBARS_STANDARD:
			g_gui.ShowToolbar(TOOLBAR_STANDARD, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_STANDARD, event.IsChecked());
			break;
		default:
			break;
	}
}

void ViewSettingsHandler::OnToggleAutomagic(wxCommandEvent& WXUNUSED(event)) {
	using namespace MenuBar;
	g_settings.setInteger(Config::USE_AUTOMAGIC, menuBar->IsItemChecked(AUTOMAGIC));
	g_settings.setInteger(Config::BORDER_IS_GROUND, menuBar->IsItemChecked(AUTOMAGIC));
	if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		g_gui.SetStatusText("Automagic enabled.");
	} else {
		g_gui.SetStatusText("Automagic disabled.");
	}
}

void ViewSettingsHandler::OnSelectionTypeChange(wxCommandEvent& WXUNUSED(event)) {
	using namespace MenuBar;
	g_settings.setInteger(Config::COMPENSATED_SELECT, menuBar->IsItemChecked(SELECT_MODE_COMPENSATE));

	if (menuBar->IsItemChecked(SELECT_MODE_CURRENT)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	} else if (menuBar->IsItemChecked(SELECT_MODE_LOWER)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	} else if (menuBar->IsItemChecked(SELECT_MODE_VISIBLE)) {
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);
	}
}
