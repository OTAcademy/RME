//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "settings.h"
#include "gui.h"

wxMemoryDC* SpriteIconGenerator::Generate(GameSprite* sprite, SpriteSize size) {
	ASSERT(sprite->width >= 1 && sprite->height >= 1);

	const int bgshade = g_settings.getInteger(Config::ICON_BACKGROUND);

	int image_size = std::max<uint8_t>(sprite->width, sprite->height) * SPRITE_PIXELS;
	wxImage image(image_size, image_size);
	image.Clear(bgshade);

	for (uint8_t l = 0; l < sprite->layers; l++) {
		for (uint8_t w = 0; w < sprite->width; w++) {
			for (uint8_t h = 0; h < sprite->height; h++) {
				const int i = sprite->getIndex(w, h, l, 0, 0, 0, 0);
				uint8_t* data = sprite->spriteList[i]->getRGBData();
				if (data) {
					wxImage img(SPRITE_PIXELS, SPRITE_PIXELS, data);
					img.SetMaskColour(0xFF, 0x00, 0xFF);
					image.Paste(img, (sprite->width - w - 1) * SPRITE_PIXELS, (sprite->height - h - 1) * SPRITE_PIXELS);
				}
			}
		}
	}

	// Now comes the resizing / antialiasing
	if (size == SPRITE_SIZE_16x16 || image.GetWidth() > SPRITE_PIXELS || image.GetHeight() > SPRITE_PIXELS) {
		int new_size = (size == SPRITE_SIZE_16x16) ? 16 : 32;
		image.Rescale(new_size, new_size);
	}

	wxBitmap bmp(image);

	// We return a new'd DC as per original contract
	return newd wxMemoryDC(bmp);
}
