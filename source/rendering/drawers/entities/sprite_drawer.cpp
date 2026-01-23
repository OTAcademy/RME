#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/core/graphics.h"
#include "sprites.h"
#include "items.h"
#include "rendering/core/batch_renderer.h"
#include "gui.h"
#include <spdlog/spdlog.h>

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

void SpriteDrawer::glBlitTexture(int sx, int sy, int texture_number, int red, int green, int blue, int alpha) {
	if (texture_number != 0) {
		float normalizedR = red / 255.0f;
		float normalizedG = green / 255.0f;
		float normalizedB = blue / 255.0f;
		float normalizedA = alpha / 255.0f;

		BatchRenderer::DrawTextureQuad(
			glm::vec2(sx, sy),
			glm::vec2(TileSize, TileSize),
			glm::vec4(normalizedR, normalizedG, normalizedB, normalizedA),
			static_cast<GLuint>(texture_number)
		);
		s_blitCount++;
	} else {
		s_zeroTextureCount++;
	}
}

void SpriteDrawer::glBlitAtlasQuad(int sx, int sy, const AtlasRegion* region, int red, int green, int blue, int alpha) {
	if (region) {
		float normalizedR = red / 255.0f;
		float normalizedG = green / 255.0f;
		float normalizedB = blue / 255.0f;
		float normalizedA = alpha / 255.0f;

		BatchRenderer::DrawAtlasQuad(
			glm::vec2(sx, sy),
			glm::vec2(TileSize, TileSize),
			glm::vec4(normalizedR, normalizedG, normalizedB, normalizedA),
			region
		);
	}
}

void SpriteDrawer::glBlitSquare(int sx, int sy, int red, int green, int blue, int alpha, int size) {
	if (size == 0) {
		size = TileSize;
	}

	float normalizedR = red / 255.0f;
	float normalizedG = green / 255.0f;
	float normalizedB = blue / 255.0f;
	float normalizedA = alpha / 255.0f;

	BatchRenderer::DrawQuad(
		glm::vec2(sx, sy),
		glm::vec2(size, size),
		glm::vec4(normalizedR, normalizedG, normalizedB, normalizedA)
	);
}

void SpriteDrawer::glSetColor(wxColor color) {
	// Not needed with BatchRenderer automatic color handling in DrawQuad,
	// but if used for stateful drawing elsewhere, we might need a state setter.
	// For now, ignoring as glBlitTexture/Square takes explicit color.
}

void SpriteDrawer::BlitSprite(int screenx, int screeny, uint32_t spriteid, int red, int green, int blue, int alpha) {
	GameSprite* spr = g_items[spriteid].sprite;
	if (spr == nullptr) {
		return;
	}
	// Call the pointer overload
	BlitSprite(screenx, screeny, spr, red, green, blue, alpha);
}

void SpriteDrawer::BlitSprite(int screenx, int screeny, GameSprite* spr, int red, int green, int blue, int alpha) {
	if (spr == nullptr) {
		return;
	}
	screenx -= spr->getDrawOffset().first;
	screeny -= spr->getDrawOffset().second;

	int tme = 0; // GetTime() % itype->FPA;

	// TEMPORARILY DISABLED: Atlas rendering for debugging
	// TODO: Re-enable once atlas issues are resolved
	bool atlasAvailable = false; // g_gui.gfx.hasAtlasManager();

	for (int cx = 0; cx != spr->width; ++cx) {
		for (int cy = 0; cy != spr->height; ++cy) {
			for (int cf = 0; cf != spr->layers; ++cf) {
				// Legacy texture path only
				int texnum = spr->getHardwareID(cx, cy, cf, -1, 0, 0, 0, tme);
				glBlitTexture(screenx - cx * TileSize, screeny - cy * TileSize, texnum, red, green, blue, alpha);
			}
		}
	}
}
