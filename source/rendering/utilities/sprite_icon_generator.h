//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UTILITIES_SPRITE_ICON_GENERATOR_H_
#define RME_RENDERING_UTILITIES_SPRITE_ICON_GENERATOR_H_

#include "rendering/core/graphics.h"

class SpriteIconGenerator {
public:
	static wxBitmap Generate(GameSprite* sprite, SpriteSize size);
	static wxBitmap Generate(GameSprite* sprite, SpriteSize size, const Outfit& outfit);
};

#endif
