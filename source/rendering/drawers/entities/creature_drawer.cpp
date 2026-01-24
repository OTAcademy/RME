//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "creature.h"
#include "gui.h"
#include "items.h"
#include "sprites.h"
#include "rendering/core/sprite_batch.h"

CreatureDrawer::CreatureDrawer() {
}

CreatureDrawer::~CreatureDrawer() {
}

void CreatureDrawer::BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Creature* c, int red, int green, int blue, int alpha, bool ingame) {
	if (!ingame && c->isSelected()) {
		red /= 2;
		green /= 2;
		blue /= 2;
	}
	BlitCreature(sprite_batch, sprite_drawer, screenx, screeny, c->getLookType(), c->getDirection(), red, green, blue, alpha);
}

void CreatureDrawer::BlitCreature(SpriteBatch& sprite_batch, SpriteDrawer* sprite_drawer, int screenx, int screeny, const Outfit& outfit, Direction dir, int red, int green, int blue, int alpha) {
	if (outfit.lookItem != 0) {
		ItemType& it = g_items[outfit.lookItem];
		sprite_drawer->BlitSprite(sprite_batch, screenx, screeny, it.sprite, red, green, blue, alpha);
	} else {
		// get outfit sprite
		GameSprite* spr = g_gui.gfx.getCreatureSprite(outfit.lookType);
		if (!spr || outfit.lookType == 0) {
			return;
		}

		int tme = 0; // GetTime() % itype->FPA;

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
						const AtlasRegion* region = mountSpr->getAtlasRegion(cx, cy, (int)dir, 0, 0, mountOutfit, tme);
						if (region) {
							sprite_drawer->glBlitAtlasQuad(sprite_batch, screenx - cx * TileSize, screeny - cy * TileSize, region, red, green, blue, alpha);
						}
					}
				}

				pattern_z = std::min<int>(1, spr->pattern_z - 1);
			}
		}

		// pattern_y => creature addon
		for (int pattern_y = 0; pattern_y < spr->pattern_y; pattern_y++) {

			// continue if we dont have this addon
			if (pattern_y > 0 && !(outfit.lookAddon & (1 << (pattern_y - 1)))) {
				continue;
			}

			for (int cx = 0; cx != spr->width; ++cx) {
				for (int cy = 0; cy != spr->height; ++cy) {
					const AtlasRegion* region = spr->getAtlasRegion(cx, cy, (int)dir, pattern_y, pattern_z, outfit, tme);
					if (region) {
						sprite_drawer->glBlitAtlasQuad(sprite_batch, screenx - cx * TileSize, screeny - cy * TileSize, region, red, green, blue, alpha);
					}
				}
			}
		}
	}
}
