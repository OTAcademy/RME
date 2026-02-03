#include "app/main.h"
#include "rendering/core/sprite_preloader.h"
#include "editor/editor.h"
#include "map/map.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/items.h"
#include "game/creature.h"
#include "game/outfit.h"
#include "ui/gui.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/graphics.h"
#include <spdlog/spdlog.h>
#include <vector>
#include <list>

namespace SpritePreloader {

	void collectTileSprites(Tile* tile, int map_x, int map_y, int map_z, const DrawingOptions& options) {
		if (!tile) {
			return;
		}

		// 1. Ground
		if (tile->ground) {
			GameSprite* spr = g_items[tile->ground->getID()].sprite;
			if (spr) {
				int frame = tile->ground->getFrame();
				int pattern_x = map_x % spr->pattern_x;
				int pattern_y = map_y % spr->pattern_y;
				int pattern_z = map_z % spr->pattern_z;

				// Touch all layers/sub-cells to ensure they are in atlas
				for (int cx = 0; cx < spr->width; ++cx) {
					for (int cy = 0; cy < spr->height; ++cy) {
						for (int cf = 0; cf < spr->layers; ++cf) {
							spr->getAtlasRegion(cx, cy, cf, -1, pattern_x, pattern_y, pattern_z, frame);
						}
					}
				}
			}
		}

		// 2. Items
		for (Item* item : tile->items) {
			GameSprite* spr = g_items[item->getID()].sprite;
			if (spr) {
				int frame = item->getFrame();
				int pattern_x = map_x % spr->pattern_x;
				int pattern_y = map_y % spr->pattern_y;
				int pattern_z = map_z % spr->pattern_z;

				// Subtype calculation (similar to ItemDrawer)
				int subtype = -1;
				ItemType& it = g_items[item->getID()];
				if (it.isSplash() || it.isFluidContainer()) {
					subtype = item->getSubtype();
				} else if (it.stackable) {
					int s = item->getSubtype();
					if (s <= 1) {
						subtype = 0;
					} else if (s <= 2) {
						subtype = 1;
					} else if (s <= 3) {
						subtype = 2;
					} else if (s <= 4) {
						subtype = 3;
					} else if (s < 10) {
						subtype = 4;
					} else if (s < 25) {
						subtype = 5;
					} else if (s < 50) {
						subtype = 6;
					} else {
						subtype = 7;
					}
				}

				for (int cx = 0; cx < spr->width; ++cx) {
					for (int cy = 0; cy < spr->height; ++cy) {
						for (int cf = 0; cf < spr->layers; ++cf) {
							spr->getAtlasRegion(cx, cy, cf, subtype, pattern_x, pattern_y, pattern_z, frame);
						}
					}
				}
			}
		}

		// 3. Creature
		if (tile->creature && options.show_creatures) {
			const Outfit& outfit = tile->creature->getLookType();
			Direction dir = tile->creature->getDirection();
			// We use 0 as animation phase for preloading, or try to guess.
			// Touching at least one frame ensures base sprite is loaded.
			int animationPhase = 0;

			if (outfit.lookItem != 0) {
				ItemType& it = g_items[outfit.lookItem];
				if (it.sprite) {
					it.sprite->getAtlasRegion(0, 0, 0, -1, 0, 0, 0, 0);
				}
			} else {
				GameSprite* spr = g_gui.gfx.getCreatureSprite(outfit.lookType);
				if (spr && outfit.lookType != 0) {
					// Mount
					if (outfit.lookMount != 0) {
						if (GameSprite* mountSpr = g_gui.gfx.getCreatureSprite(outfit.lookMount)) {
							Outfit mountOutfit = outfit;
							mountOutfit.lookType = outfit.lookMount;
							for (int cx = 0; cx < mountSpr->width; ++cx) {
								for (int cy = 0; cy < mountSpr->height; ++cy) {
									mountSpr->getAtlasRegion(cx, cy, (int)dir, 0, 0, mountOutfit, animationPhase);
								}
							}
						}
					}
					// Body and Addons
					for (int pattern_y = 0; pattern_y < spr->pattern_y; ++pattern_y) {
						if (pattern_y > 0 && !(outfit.lookAddon & (1 << (pattern_y - 1)))) {
							continue;
						}
						for (int cx = 0; cx < spr->width; ++cx) {
							for (int cy = 0; cy < spr->height; ++cy) {
								spr->getAtlasRegion(cx, cy, (int)dir, pattern_y, 0, outfit, animationPhase);
							}
						}
					}
				}
			}
		}
	}

	void PreloadVisibleSprites(Editor* editor, const RenderView& view, const DrawingOptions& options) {
		if (!g_gui.gfx.ensureAtlasManager()) {
			return;
		}

		// View bounds in tiles
		int start_x = view.start_x;
		int start_y = view.start_y;
		int end_x = view.end_x;
		int end_y = view.end_y;
		int z = view.floor;

		// Optimization: Visit nodes once
		editor->map.visitLeaves(start_x, start_y, end_x, end_y, [&](MapNode* nd, int nd_x, int nd_y) {
			// Only preload if node is visible/requested (for live client)
			if (!nd->isVisible(z > GROUND_LAYER)) {
				return;
			}

			for (int lx = 0; lx < 4; ++lx) {
				for (int ly = 0; ly < 4; ++ly) {
					int map_x = nd_x + lx;
					int map_y = nd_y + ly;

					// Quick viewport check
					int dummy_x, dummy_y;
					if (!view.IsTileVisible(map_x, map_y, z, dummy_x, dummy_y)) {
						continue;
					}

					TileLocation* loc = nd->getTile(lx, ly, z);
					if (loc && loc->get()) {
						collectTileSprites(loc->get(), map_x, map_y, z, options);
					}

					// Also preload lower floor if transparent?
					// For now, stick to current floor to keep it fast.
				}
			}
		});

		// At very high zoom or large displays, this might be called many times.
		// spdlog::debug("SpritePreloader::PreloadVisibleSprites finished");
	}

} // namespace SpritePreloader
