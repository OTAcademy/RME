//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_ITEM_DRAWER_H_
#define RME_RENDERING_ITEM_DRAWER_H_

#include "definitions.h"
#include "position.h"

// Forward declarations
class SpriteDrawer;
class CreatureDrawer;
class Tile;
class Item;
class ItemType;
class ItemType;
struct DrawingOptions;
class SpriteBatch;
class PrimitiveRenderer;

class ItemDrawer {
public:
	ItemDrawer();
	~ItemDrawer();

	void BlitItem(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, int& draw_x, int& draw_y, const Tile* tile, Item* item, const DrawingOptions& options, bool ephemeral = false, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitItem(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, int& draw_x, int& draw_y, const Position& pos, Item* item, const DrawingOptions& options, bool ephemeral = false, int red = 255, int green = 255, int blue = 255, int alpha = 255, const Tile* tile = nullptr);

	void DrawRawBrush(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, ItemType* itemType, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
	void DrawHookIndicator(PrimitiveRenderer& primitive_renderer, int x, int y, const ItemType& type);
};

#endif
