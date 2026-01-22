#include "main.h"
#include "rendering/drawing_options.h"

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

bool DrawingOptions::isDrawLight() const noexcept {
	return show_lights;
}
