//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_BRUSH_OVERLAY_DRAWER_H_
#define RME_RENDERING_BRUSH_OVERLAY_DRAWER_H_

#include "position.h"
#include <wx/colour.h>

class MapDrawer;
struct RenderView;
struct DrawingOptions;
class Brush;
class Editor;

class ItemDrawer;
class SpriteDrawer;
class CreatureDrawer;

class BrushOverlayDrawer {
public:
	BrushOverlayDrawer();
	~BrushOverlayDrawer();

	void draw(MapDrawer* drawer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options, Editor& editor);

private:
	enum BrushColor {
		COLOR_BRUSH,
		COLOR_HOUSE_BRUSH,
		COLOR_FLAG_BRUSH,
		COLOR_SPAWN_BRUSH,
		COLOR_ERASER,
		COLOR_VALID,
		COLOR_INVALID,
		COLOR_BLANK,
	};

	void get_color(Brush* brush, Editor& editor, const Position& position, uint8_t& r, uint8_t& g, uint8_t& b);
	void set_gl_color(BrushColor color);
	void set_gl_color_check(Brush* brush, Editor& editor, const Position& pos);
};

#endif
