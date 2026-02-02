//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UTILITIES_SPRITE_ICON_GENERATOR_H_
#define RME_RENDERING_UTILITIES_SPRITE_ICON_GENERATOR_H_

#include "rendering/core/graphics.h"
#include "game/creature.h"

class SpriteIconGenerator {
public:
	static wxBitmap Generate(GameSprite* sprite, SpriteSize size, bool rescale = true);
	static wxBitmap Generate(GameSprite* sprite, SpriteSize size, const Outfit& outfit, bool rescale = true, Direction direction = SOUTH);
};

#endif
