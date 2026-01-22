//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_SPRITE_DRAWER_H_
#define RME_RENDERING_SPRITE_DRAWER_H_

#include <wx/colour.h>
#include "definitions.h"

// Forward declarations
class GameSprite;

class SpriteDrawer {
public:
	SpriteDrawer();
	~SpriteDrawer();

	void ResetCache();

	void glBlitTexture(int sx, int sy, int texture_number, int red, int green, int blue, int alpha);
	void glBlitSquare(int sx, int sy, int red, int green, int blue, int alpha, int size = 0);
	void glSetColor(wxColor color);

	void BlitSprite(int screenx, int screeny, uint32_t spriteid, int red = 255, int green = 255, int blue = 255, int alpha = 255);
	void BlitSprite(int screenx, int screeny, GameSprite* spr, int red = 255, int green = 255, int blue = 255, int alpha = 255);

private:
	// Texture bind caching for performance
	GLuint last_bound_texture_ = 0;
};

#endif
