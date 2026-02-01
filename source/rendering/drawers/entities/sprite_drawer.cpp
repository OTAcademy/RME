#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/core/graphics.h"
#include "game/sprites.h"
#include "game/items.h"

#include "ui/gui.h"
#include <spdlog/spdlog.h>
#include "rendering/core/sprite_batch.h"
#include "rendering/core/atlas_manager.h"

static int s_blitCount = 0;
static int s_zeroTextureCount = 0;

SpriteDrawer::SpriteDrawer() :
	last_bound_texture_(0) {
}

SpriteDrawer::~SpriteDrawer() {
}

void SpriteDrawer::ResetCache() {
	last_bound_texture_ = 0;
	// Log stats each frame
	if (s_blitCount > 0) {
		spdlog::info("SpriteDrawer: {} sprites drawn, {} zero-texture skipped", s_blitCount, s_zeroTextureCount);
	}
	s_blitCount = 0;
	s_zeroTextureCount = 0;
}

void SpriteDrawer::glBlitAtlasQuad(SpriteBatch& sprite_batch, int sx, int sy, const AtlasRegion* region, int red, int green, int blue, int alpha) {
	if (region) {
		float normalizedR = red / 255.0f;
		float normalizedG = green / 255.0f;
		float normalizedB = blue / 255.0f;
		float normalizedA = alpha / 255.0f;

		sprite_batch.draw(
			(float)sx, (float)sy,
			(float)TileSize, (float)TileSize,
			*region,
			normalizedR, normalizedG, normalizedB, normalizedA
		);
	}
}

void SpriteDrawer::glBlitSquare(SpriteBatch& sprite_batch, int sx, int sy, int red, int green, int blue, int alpha, int size) {
	if (size == 0) {
		size = TileSize;
	}

	float normalizedR = red / 255.0f;
	float normalizedG = green / 255.0f;
	float normalizedB = blue / 255.0f;
	float normalizedA = alpha / 255.0f;

	// Use Graphics::getAtlasManager() to get the atlas manager for white pixel access
	// This assumes Graphics and AtlasManager are available
	if (g_gui.gfx.hasAtlasManager()) {
		sprite_batch.drawRect((float)sx, (float)sy, (float)size, (float)size, glm::vec4(normalizedR, normalizedG, normalizedB, normalizedA), *g_gui.gfx.getAtlasManager());
	}
}

void SpriteDrawer::glDrawBox(SpriteBatch& sprite_batch, int sx, int sy, int width, int height, int red, int green, int blue, int alpha) {
	float normalizedR = red / 255.0f;
	float normalizedG = green / 255.0f;
	float normalizedB = blue / 255.0f;
	float normalizedA = alpha / 255.0f;

	if (g_gui.gfx.hasAtlasManager()) {
		sprite_batch.drawRectLines((float)sx, (float)sy, (float)width, (float)height, glm::vec4(normalizedR, normalizedG, normalizedB, normalizedA), *g_gui.gfx.getAtlasManager());
	}
}

void SpriteDrawer::glSetColor(wxColor color) {
	// Not needed with BatchRenderer automatic color handling in DrawQuad,
	// but if used for stateful drawing elsewhere, we might need a state setter.
	// For now, ignoring as glBlitTexture/Square takes explicit color.
}

void SpriteDrawer::BlitSprite(SpriteBatch& sprite_batch, int screenx, int screeny, uint32_t spriteid, int red, int green, int blue, int alpha) {
	GameSprite* spr = g_items[spriteid].sprite;
	if (spr == nullptr) {
		return;
	}
	// Call the pointer overload
	BlitSprite(sprite_batch, screenx, screeny, spr, red, green, blue, alpha);
}

void SpriteDrawer::BlitSprite(SpriteBatch& sprite_batch, int screenx, int screeny, GameSprite* spr, int red, int green, int blue, int alpha) {
	if (spr == nullptr) {
		return;
	}
	screenx -= spr->getDrawOffset().first;
	screeny -= spr->getDrawOffset().second;

	int tme = 0; // GetTime() % itype->FPA;

	// Atlas-only rendering - ensure atlas is available
	// Note: ensureAtlasManager is called by MapDrawer at frame start usually, but we check here too if needed?
	// BatchRenderer::SetAtlasManager call removed. Use sprite_batch.

	for (int cx = 0; cx != spr->width; ++cx) {
		for (int cy = 0; cy != spr->height; ++cy) {
			for (int cf = 0; cf != spr->layers; ++cf) {
				const AtlasRegion* region = spr->getAtlasRegion(cx, cy, cf, -1, 0, 0, 0, tme);
				if (region) {
					glBlitAtlasQuad(sprite_batch, screenx - cx * TileSize, screeny - cy * TileSize, region, red, green, blue, alpha);
				}
				// No fallback - if region is null, sprite failed to load
			}
		}
	}
}
