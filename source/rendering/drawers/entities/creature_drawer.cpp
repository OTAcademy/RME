//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

// glut include removed

#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "game/creature.h"
#include "ui/gui.h"
#include "game/items.h"
#include "game/sprites.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/game_sprite.h"
#include "rendering/core/animator.h"
#include <spdlog/spdlog.h>

CreatureDrawer::CreatureDrawer() {
}

CreatureDrawer::~CreatureDrawer() {
}

void CreatureDrawer::BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Creature* c, int red, int green, int blue, int alpha, bool ingame, int animationPhase) {
	if (!ingame && c->isSelected()) {
		red /= 2;
		green /= 2;
		blue /= 2;
	}
	BlitCreature(sprite_batch, sprite_drawer, screenx, screeny, c->getLookType(), c->getDirection(), red, green, blue, alpha, animationPhase);
}

void CreatureDrawer::BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Outfit& outfit, Direction dir, int red, int green, int blue, int alpha, int animationPhase) {
	if (outfit.lookItem != 0) {
		ItemType& it = g_items[outfit.lookItem];
		sprite_drawer->BlitSprite(sprite_batch, screenx, screeny, it.sprite, red, green, blue, alpha);
	} else {
		// get outfit sprite
		GameSprite* spr = g_gui.gfx.getCreatureSprite(outfit.lookType);
		if (!spr || outfit.lookType == 0) {
			return;
		}

		// Resolve animation frame for walk animation
		// For in-game preview: animationPhase controls walk animation
		// - When > 0: walking (use the provided animation phase)
		// - When == 0: standing idle (ALWAYS use frame 0, NOT the global animator)
		// The global animator is for idle creatures on the map, NOT for the player
		int resolvedFrame = 0;
		if (animationPhase > 0) {
			// Walking: use the calculated walk animation phase
			resolvedFrame = animationPhase;
		} else {
			// Standing still: always use frame 0 (idle)
			// Do NOT use spr->animator->getFrame() - that's for global idle animations
			// of creatures on the map, not for the player character
			resolvedFrame = 0;
		}
		

		// mount and addon drawing thanks to otc code
		// mount colors by Zbizu
		int pattern_z = 0;
		if (outfit.lookMount != 0) {
			if (GameSprite* mountSpr = g_gui.gfx.getCreatureSprite(outfit.lookMount)) {
				// generate mount colors
				Outfit mountOutfit;
				mountOutfit.lookType = outfit.lookMount;
				mountOutfit.lookHead = outfit.lookMountHead;
				mountOutfit.lookBody = outfit.lookMountBody;
				mountOutfit.lookLegs = outfit.lookMountLegs;
				mountOutfit.lookFeet = outfit.lookMountFeet;

				for (int cx = 0; cx != mountSpr->width; ++cx) {
					for (int cy = 0; cy != mountSpr->height; ++cy) {
						const AtlasRegion* region = mountSpr->getAtlasRegion(cx, cy, (int)dir, 0, 0, mountOutfit, resolvedFrame);
						if (region) {
							sprite_drawer->glBlitAtlasQuad(sprite_batch, screenx - cx * TileSize - mountSpr->getDrawOffset().first, screeny - cy * TileSize - mountSpr->getDrawOffset().second, region, red, green, blue, alpha);
						}
					}
				}

				pattern_z = std::min<int>(1, spr->pattern_z - 1);
			}
		}

		// pattern_y => creature addon
		for (int pattern_y = 0; pattern_y < spr->pattern_y; pattern_y++) {

			// continue if we dont have this addon
			if (pattern_y > 0) {
				if ((pattern_y - 1 >= 31) || !(outfit.lookAddon & (1 << (pattern_y - 1)))) {
					continue;
				}
			}

			for (int cx = 0; cx != spr->width; ++cx) {
				for (int cy = 0; cy != spr->height; ++cy) {
					const AtlasRegion* region = spr->getAtlasRegion(cx, cy, (int)dir, pattern_y, pattern_z, outfit, resolvedFrame);
					if (region) {
						sprite_drawer->glBlitAtlasQuad(sprite_batch, screenx - cx * TileSize - spr->getDrawOffset().first, screeny - cy * TileSize - spr->getDrawOffset().second, region, red, green, blue, alpha);
					}
				}
			}
		}
	}
}
