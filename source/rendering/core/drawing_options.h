#ifndef RME_RENDERING_DRAWING_OPTIONS_H_
#define RME_RENDERING_DRAWING_OPTIONS_H_

#include <cstdint>
#include <wx/wx.h>

struct DrawingOptions {
	DrawingOptions();

	void SetIngame();
	void SetDefault();
	void Update();
	bool isDrawLight() const noexcept;

	bool transparent_floors;
	bool transparent_items;
	bool show_ingame_box;
	bool show_lights;
	bool show_light_str;
	bool show_tech_items;
	bool show_waypoints;
	bool ingame;
	bool dragging;
	bool boundbox_selection;

	int show_grid;
	bool show_all_floors;
	bool show_creatures;
	bool show_spawns;
	bool show_houses;
	bool show_shade;
	bool show_special_tiles;
	bool show_items;

	bool highlight_items;
	bool highlight_locked_doors;
	bool show_blocking;
	bool show_tooltips;

	bool show_as_minimap;
	bool show_only_colors;
	bool show_only_modified;
	bool show_preview;
	bool show_hooks;
	bool hide_items_when_zoomed;
	bool show_towns;
	bool always_show_zones;
	bool extended_house_shader;

	bool experimental_fog;

	uint32_t current_house_id;
	wxColor global_light_color;
	float light_intensity;
	float ambient_light_level;
};

#endif
