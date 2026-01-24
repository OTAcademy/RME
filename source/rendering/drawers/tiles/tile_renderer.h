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
struct LightBuffer;
struct LightBuffer;
class SpriteBatch;
class PrimitiveRenderer;

class TileRenderer {
public:
	TileRenderer(ItemDrawer* id, SpriteDrawer* sd, CreatureDrawer* cd, FloorDrawer* fd, MarkerDrawer* md, TooltipDrawer* td, Editor* ed);

	void DrawTile(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, TileLocation* location, const RenderView& view, const DrawingOptions& options, uint32_t current_house_id, std::ostringstream& tooltip_stream);
	void AddLight(TileLocation* location, const RenderView& view, const DrawingOptions& options, LightBuffer& light_buffer);

private:
	ItemDrawer* item_drawer;
	SpriteDrawer* sprite_drawer;
	CreatureDrawer* creature_drawer;
	FloorDrawer* floor_drawer;
	MarkerDrawer* marker_drawer;
	TooltipDrawer* tooltip_drawer;
	Editor* editor;
};

#endif
