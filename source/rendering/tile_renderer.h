#ifndef RME_RENDERING_TILE_RENDERER_H_
#define RME_RENDERING_TILE_RENDERER_H_

#include <memory>
#include <sstream>
#include <stdint.h>

class TileLocation;
struct RenderView;
struct DrawingOptions;
class Editor;
class ItemDrawer;
class SpriteDrawer;
class CreatureDrawer;
class FloorDrawer;
class MarkerDrawer;
class TooltipDrawer;
class LightDrawer;

class TileRenderer {
	ItemDrawer* item_drawer;
	SpriteDrawer* sprite_drawer;
	CreatureDrawer* creature_drawer;
	FloorDrawer* floor_drawer;
	MarkerDrawer* marker_drawer;
	TooltipDrawer* tooltip_drawer;
	LightDrawer* light_drawer;
	Editor* editor;

public:
	TileRenderer(ItemDrawer* id, SpriteDrawer* sd, CreatureDrawer* cd, FloorDrawer* fd, MarkerDrawer* md, TooltipDrawer* td, LightDrawer* ld, Editor* ed);
	void DrawTile(TileLocation* location, const RenderView& view, const DrawingOptions& options, uint32_t current_house_id, std::ostringstream& tooltip_stream);
	void AddLight(TileLocation* location, const RenderView& view, const DrawingOptions& options);
};

#endif
