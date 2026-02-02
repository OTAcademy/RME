//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "app/settings.h"
#include "ui/gui.h"

wxBitmap SpriteIconGenerator::Generate(GameSprite* sprite, SpriteSize size, bool rescale) {
	ASSERT(sprite->width >= 1 && sprite->height >= 1);

	const int bgshade = g_settings.getInteger(Config::ICON_BACKGROUND);

	int image_size = std::max<uint8_t>(sprite->width, sprite->height) * SPRITE_PIXELS;
	wxImage image(image_size, image_size);
	image.Create(image_size, image_size);
	image.InitAlpha();
	for (int i = 0; i < image_size * image_size; ++i) {
		image.SetRGB(i % image_size, i / image_size, (bgshade >> 16) & 0xFF, (bgshade >> 8) & 0xFF, bgshade & 0xFF);
		image.SetAlpha(i % image_size, i / image_size, 255);
	}

	for (uint8_t l = 0; l < sprite->layers; l++) {
		for (uint8_t w = 0; w < sprite->width; w++) {
			for (uint8_t h = 0; h < sprite->height; h++) {
				const int i = sprite->getIndex(w, h, l, 0, 0, 0, 0);
				std::unique_ptr<uint8_t[]> data = sprite->spriteList[i]->getRGBData();
				if (data) {
					wxImage img(SPRITE_PIXELS, SPRITE_PIXELS, data.get(), true);
					img.SetMaskColour(0xFF, 0x00, 0xFF);
					image.Paste(img, (sprite->width - w - 1) * SPRITE_PIXELS, (sprite->height - h - 1) * SPRITE_PIXELS);
				}
			}
		}
	}

	// Now comes the resizing / antialiasing
	if (rescale && (size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_64x64 || image.GetWidth() > SPRITE_PIXELS || image.GetHeight() > SPRITE_PIXELS)) {
		int new_size = 32;
		if (size == SPRITE_SIZE_16x16) {
			new_size = 16;
		} else if (size == SPRITE_SIZE_64x64) {
			new_size = 64;
		}
		image.Rescale(new_size, new_size, wxIMAGE_QUALITY_HIGH);
	}

	return wxBitmap(image);
}

wxBitmap SpriteIconGenerator::Generate(GameSprite* sprite, SpriteSize size, const Outfit& outfit, bool rescale, Direction direction) {
	ASSERT(sprite->width >= 1 && sprite->height >= 1);

	const int bgshade = g_settings.getInteger(Config::ICON_BACKGROUND);

	int image_size = std::max<uint8_t>(sprite->width, sprite->height) * SPRITE_PIXELS;
	wxImage image(image_size, image_size);
	image.Create(image_size, image_size);
	image.InitAlpha();
	for (int i = 0; i < image_size * image_size; ++i) {
		image.SetRGB(i % image_size, i / image_size, (bgshade >> 16) & 0xFF, (bgshade >> 8) & 0xFF, bgshade & 0xFF);
		image.SetAlpha(i % image_size, i / image_size, 255);
	}

	int frame_index = 0;
	if (sprite->pattern_x == 4) {
		frame_index = direction;
	}

	// Mounts
	int pattern_z = 0;
	if (outfit.lookMount != 0) {
		if (GameSprite* mountSpr = g_gui.gfx.getCreatureSprite(outfit.lookMount)) {
			// Mount outfit
			Outfit mountOutfit;
			mountOutfit.lookType = outfit.lookMount;
			mountOutfit.lookHead = outfit.lookMountHead;
			mountOutfit.lookBody = outfit.lookMountBody;
			mountOutfit.lookLegs = outfit.lookMountLegs;
			mountOutfit.lookFeet = outfit.lookMountFeet;

			// We need to render the mount
			// Simplified rendering: just render base frame 0 for mount (or south)
			int mount_frame_index = 0;
			if (mountSpr->pattern_x == 4) {
				mount_frame_index = direction;
			}

			for (uint8_t l = 0; l < mountSpr->layers; l++) {
				for (uint8_t w = 0; w < mountSpr->width; w++) {
					for (uint8_t h = 0; h < mountSpr->height; h++) {
						std::unique_ptr<uint8_t[]> data = nullptr;
						// Handle mount sprite layers/templates similar to main sprite
						// (Usually mounts are standard creatures)
						if (mountSpr->layers == 2) {
							if (l == 1) {
								continue;
							}
							data = mountSpr->getTemplateImage(mountSpr->getIndex(w, h, 0, mount_frame_index, 0, 0, 0), mountOutfit)->getRGBData();
						} else {
							// Standard mount
							data = mountSpr->spriteList[mountSpr->getIndex(w, h, l, mount_frame_index, 0, 0, 0)]->getRGBData();
						}

						if (data) {
							wxImage img(SPRITE_PIXELS, SPRITE_PIXELS, data.get(), true);
							img.SetMaskColour(0xFF, 0x00, 0xFF);
							// Mount offset
							int mount_x = (sprite->width - w - 1) * SPRITE_PIXELS - mountSpr->getDrawOffset().first;
							int mount_y = (sprite->height - h - 1) * SPRITE_PIXELS - mountSpr->getDrawOffset().second;
							image.Paste(img, mount_x, mount_y);
						}
					}
				}
			}
			pattern_z = std::min<int>(1, sprite->pattern_z - 1);
		}
	}

	for (int pattern_y = 0; pattern_y < sprite->pattern_y; pattern_y++) {
		if (pattern_y > 0) {
			if ((pattern_y - 1 >= 31) || !(outfit.lookAddon & (1 << (pattern_y - 1)))) {
				continue;
			}
		}

		for (uint8_t l = 0; l < sprite->layers; l++) {
			for (uint8_t w = 0; w < sprite->width; w++) {
				for (uint8_t h = 0; h < sprite->height; h++) {
					std::unique_ptr<uint8_t[]> data = nullptr;

					if (sprite->layers == 2) {
						if (l == 1) {
							continue;
						}
						data = sprite->getTemplateImage(sprite->getIndex(w, h, 0, frame_index, pattern_y, pattern_z, 0), outfit)->getRGBData();
					} else if (sprite->layers == 4) {
						if (l == 1 || l == 3) {
							continue;
						}
						if (l == 0) {
							data = sprite->getTemplateImage(sprite->getIndex(w, h, 0, frame_index, pattern_y, pattern_z, 0), outfit)->getRGBData();
						}
						if (l == 2) {
							data = sprite->getTemplateImage(sprite->getIndex(w, h, 2, frame_index, pattern_y, pattern_z, 0), outfit)->getRGBData();
						}
					} else {
						data = sprite->spriteList[sprite->getIndex(w, h, l, frame_index, pattern_y, pattern_z, 0)]->getRGBData();
					}

					if (data) {
						wxImage img(SPRITE_PIXELS, SPRITE_PIXELS, data.get(), true);
						img.SetMaskColour(0xFF, 0x00, 0xFF);
						image.Paste(img, (sprite->width - w - 1) * SPRITE_PIXELS, (sprite->height - h - 1) * SPRITE_PIXELS);
					}
				}
			}
		}
	}

	// Now comes the resizing / antialiasing
	if (rescale && (size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_64x64 || image.GetWidth() > SPRITE_PIXELS || image.GetHeight() > SPRITE_PIXELS)) {
		int new_size = 32;
		if (size == SPRITE_SIZE_16x16) {
			new_size = 16;
		} else if (size == SPRITE_SIZE_64x64) {
			new_size = 64;
		}
		image.Rescale(new_size, new_size, wxIMAGE_QUALITY_HIGH);
	}

	return wxBitmap(image);
}
