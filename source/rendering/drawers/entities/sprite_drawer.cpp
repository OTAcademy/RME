//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/entities/sprite_drawer.h"
#include "rendering/core/graphics.h"
#include "sprites.h"
#include "items.h"

SpriteDrawer::SpriteDrawer() : last_bound_texture_(0) {
}

SpriteDrawer::~SpriteDrawer() {
}

void SpriteDrawer::ResetCache() {
	last_bound_texture_ = 0;
}

void SpriteDrawer::glBlitTexture(int sx, int sy, int texture_number, int red, int green, int blue, int alpha) {
	if (texture_number != 0) {
		// Cache texture binding to avoid redundant calls
		if (static_cast<GLuint>(texture_number) != last_bound_texture_) {
			glBindTexture(GL_TEXTURE_2D, texture_number);
			last_bound_texture_ = texture_number;
		}
		glColor4ub(uint8_t(red), uint8_t(green), uint8_t(blue), uint8_t(alpha));
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);
		glVertex2f(sx, sy);
		glTexCoord2f(1.f, 0.f);
		glVertex2f(sx + TileSize, sy);
		glTexCoord2f(1.f, 1.f);
		glVertex2f(sx + TileSize, sy + TileSize);
		glTexCoord2f(0.f, 1.f);
		glVertex2f(sx, sy + TileSize);
		glEnd();
	}
}

void SpriteDrawer::glBlitSquare(int sx, int sy, int red, int green, int blue, int alpha, int size) {
	if (size == 0) {
		size = TileSize;
	}

	glColor4ub(uint8_t(red), uint8_t(green), uint8_t(blue), uint8_t(alpha));
	glBegin(GL_QUADS);
	glVertex2f(sx, sy);
	glVertex2f(sx + size, sy);
	glVertex2f(sx + size, sy + size);
	glVertex2f(sx, sy + size);
	glEnd();
}

void SpriteDrawer::glSetColor(wxColor color) {
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
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
	for (int cx = 0; cx != spr->width; ++cx) {
		for (int cy = 0; cy != spr->height; ++cy) {
			for (int cf = 0; cf != spr->layers; ++cf) {
				int texnum = spr->getHardwareID(cx, cy, cf, -1, 0, 0, 0, tme);
				glBlitTexture(screenx - cx * TileSize, screeny - cy * TileSize, texnum, red, green, blue, alpha);
			}
		}
	}
}
