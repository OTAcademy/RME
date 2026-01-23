//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_CORE_EDITOR_SPRITE_H_
#define RME_RENDERING_CORE_EDITOR_SPRITE_H_

#include "rendering/core/graphics.h"

class EditorSprite : public Sprite {
public:
	EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32);
	virtual ~EditorSprite();

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1);
	virtual void unloadDC();

protected:
	wxBitmap* bm[SPRITE_SIZE_COUNT];
};

#endif
