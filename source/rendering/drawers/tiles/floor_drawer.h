//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_FLOOR_DRAWER_H_
#define RME_RENDERING_FLOOR_DRAWER_H_

class MapDrawer;
struct RenderView;
struct DrawingOptions;
class Editor;

class ItemDrawer;
class SpriteDrawer;

class CreatureDrawer;
class SpriteBatch;
class PrimitiveRenderer;

class FloorDrawer {
public:
	FloorDrawer();
	~FloorDrawer();

	void draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options, Editor& editor);
};

#endif
