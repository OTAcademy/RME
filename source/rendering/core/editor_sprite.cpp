//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/core/editor_sprite.h"

EditorSprite::EditorSprite(std::unique_ptr<wxBitmap> b16x16, std::unique_ptr<wxBitmap> b32x32) {
	bm[SPRITE_SIZE_16x16] = std::move(b16x16);
	bm[SPRITE_SIZE_32x32] = std::move(b32x32);
}

EditorSprite::~EditorSprite() {
	// Unique pointers clean themselves up, but unloadDC allows explicit clearing
	unloadDC();
}

void EditorSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height) {
	wxBitmap* sp = bm[sz].get();
	if (sp) {
		dc->DrawBitmap(*sp, start_x, start_y, true);
	}
}

void EditorSprite::unloadDC() {
	bm[SPRITE_SIZE_16x16].reset();
	bm[SPRITE_SIZE_32x32].reset();
}
