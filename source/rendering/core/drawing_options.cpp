#include "main.h"
#include "rendering/core/drawing_options.h"

DrawingOptions::DrawingOptions() {
	SetDefault();
}

void DrawingOptions::SetDefault() {
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	show_lights = false;
	show_light_str = true;
	show_tech_items = true;
	show_waypoints = true;
	ingame = false;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_spawns = true;
	show_houses = true;
	show_shade = true;
	show_special_tiles = true;
	show_items = true;

	highlight_items = false;
	highlight_locked_doors = true;
	show_blocking = false;
	show_tooltips = false;
	show_as_minimap = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	show_hooks = false;
	hide_items_when_zoomed = true;
}

void DrawingOptions::SetIngame() {
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	show_lights = false;
	show_light_str = false;
	show_tech_items = false;
	show_waypoints = false;
	ingame = true;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_spawns = false;
	show_houses = false;
	show_shade = false;
	show_special_tiles = false;
	show_items = true;

	highlight_items = false;
	highlight_locked_doors = false;
	show_blocking = false;
	show_tooltips = false;
	show_as_minimap = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	show_hooks = false;
	hide_items_when_zoomed = false;
}

#include "settings.h"

void DrawingOptions::Update() {
	transparent_floors = g_settings.getBoolean(Config::TRANSPARENT_FLOORS);
	transparent_items = g_settings.getBoolean(Config::TRANSPARENT_ITEMS);
	show_ingame_box = g_settings.getBoolean(Config::SHOW_INGAME_BOX);
	show_lights = g_settings.getBoolean(Config::SHOW_LIGHTS);
	show_light_str = g_settings.getBoolean(Config::SHOW_LIGHT_STR);
	show_tech_items = g_settings.getBoolean(Config::SHOW_TECHNICAL_ITEMS);
	show_waypoints = g_settings.getBoolean(Config::SHOW_WAYPOINTS);
	show_grid = g_settings.getInteger(Config::SHOW_GRID);
	ingame = !g_settings.getBoolean(Config::SHOW_EXTRA);
	show_all_floors = g_settings.getBoolean(Config::SHOW_ALL_FLOORS);
	show_creatures = g_settings.getBoolean(Config::SHOW_CREATURES);
	show_spawns = g_settings.getBoolean(Config::SHOW_SPAWNS);
	show_houses = g_settings.getBoolean(Config::SHOW_HOUSES);
	show_shade = g_settings.getBoolean(Config::SHOW_SHADE);
	show_special_tiles = g_settings.getBoolean(Config::SHOW_SPECIAL_TILES);
	show_items = g_settings.getBoolean(Config::SHOW_ITEMS);
	highlight_items = g_settings.getBoolean(Config::HIGHLIGHT_ITEMS);
	highlight_locked_doors = g_settings.getBoolean(Config::HIGHLIGHT_LOCKED_DOORS);
	show_blocking = g_settings.getBoolean(Config::SHOW_BLOCKING);
	show_tooltips = g_settings.getBoolean(Config::SHOW_TOOLTIPS);
	show_as_minimap = g_settings.getBoolean(Config::SHOW_AS_MINIMAP);
	show_only_colors = g_settings.getBoolean(Config::SHOW_ONLY_TILEFLAGS);
	show_only_modified = g_settings.getBoolean(Config::SHOW_ONLY_MODIFIED_TILES);
	show_preview = g_settings.getBoolean(Config::SHOW_PREVIEW);
	show_hooks = g_settings.getBoolean(Config::SHOW_WALL_HOOKS);
	hide_items_when_zoomed = g_settings.getBoolean(Config::HIDE_ITEMS_WHEN_ZOOMED);
	show_towns = g_settings.getBoolean(Config::SHOW_TOWNS);
	always_show_zones = g_settings.getBoolean(Config::ALWAYS_SHOW_ZONES);
	extended_house_shader = g_settings.getBoolean(Config::EXT_HOUSE_SHADER);

	experimental_fog = g_settings.getBoolean(Config::EXPERIMENTAL_FOG);
}

bool DrawingOptions::isDrawLight() const noexcept {
	return show_lights;
}
