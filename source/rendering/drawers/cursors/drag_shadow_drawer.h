//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_DRAG_SHADOW_DRAWER_H_
#define RME_RENDERING_DRAG_SHADOW_DRAWER_H_

class MapDrawer;
struct RenderView;
struct DrawingOptions;

class ItemDrawer;
class SpriteDrawer;
class CreatureDrawer;
class SpriteBatch;
class PrimitiveRenderer;

class DragShadowDrawer {
public:
	DragShadowDrawer();
	~DragShadowDrawer();

	void draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, MapDrawer* drawer, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, const RenderView& view, const DrawingOptions& options);
};

#endif
