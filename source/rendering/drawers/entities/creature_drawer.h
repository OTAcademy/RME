//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_CREATURE_DRAWER_H_
#define RME_RENDERING_CREATURE_DRAWER_H_

#include "definitions.h"
#include "outfit.h"
#include "creature.h"

// Forward declarations
// Forward declarations
class SpriteDrawer;
class SpriteBatch;

class CreatureDrawer {
public:
	CreatureDrawer();
	~CreatureDrawer();

	void BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Creature* c, int red = 255, int green = 255, int blue = 255, int alpha = 255, bool ingame = false);
	void BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Outfit& outfit, Direction dir, int red = 255, int green = 255, int blue = 255, int alpha = 255);
};

#endif
