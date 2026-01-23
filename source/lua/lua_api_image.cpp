//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "lua_api_image.h"
#include "../gui.h"
#include "../graphics.h"
#include "../items.h"

namespace LuaAPI {

LuaImage::LuaImage() : spriteId(0), spriteSource(false) {
	// Empty image
}

LuaImage::LuaImage(const std::string& path) : filePath(path), spriteId(0), spriteSource(false) {
	if (!path.empty()) {
		image.LoadFile(wxString(path));
	}
}

LuaImage::LuaImage(int id, bool isItemSprite) : spriteId(id), spriteSource(true) {
	if (isItemSprite) {
		// Get sprite ID from item type
		if (g_items.typeExists(id)) {
			ItemType& itemType = g_items.getItemType(id);
			if (itemType.id != 0) {
				loadFromSpriteId(itemType.clientID);
			}
		}
	} else {
		loadFromSpriteId(id);
	}
}

LuaImage::LuaImage(const LuaImage& other) :
	image(other.image.IsOk() ? other.image.Copy() : wxImage()),
	filePath(other.filePath),
	spriteId(other.spriteId),
	spriteSource(other.spriteSource) {
}

LuaImage::~LuaImage() {
	// wxImage handles its own cleanup
}

LuaImage LuaImage::loadFromFile(const std::string& path) {
	return LuaImage(path);
}

LuaImage LuaImage::loadFromItemSprite(int itemId) {
	return LuaImage(itemId, true);
}

LuaImage LuaImage::loadFromSprite(int spriteId) {
	return LuaImage(spriteId, false);
}

// Helper to reliably get sprite ID from item ID, handling client ID mapping
int getItemSpriteId(int itemId) {
	if (!g_items.typeExists(itemId)) return 0;
	ItemType& itemType = g_items.getItemType(itemId);
	return itemType.clientID;
}

void LuaImage::loadFromSpriteId(int id) {
	Sprite* sprite = g_gui.gfx.getSprite(id);
	if (!sprite) {
		return;
	}

	GameSprite* gameSprite = dynamic_cast<GameSprite*>(sprite);
	if (gameSprite && gameSprite->width > 0 && gameSprite->height > 0) {
		// Calculate full sprite size (can be larger than 32x32 for multi-tile sprites)
		int spriteWidth = gameSprite->width * 32;
		int spriteHeight = gameSprite->height * 32;

		// Create image with alpha channel
		image.Create(spriteWidth, spriteHeight);
		image.InitAlpha();

		// Fill with transparent background
		unsigned char* imgData = image.GetData();
		unsigned char* alphaData = image.GetAlpha();
		memset(imgData, 0, spriteWidth * spriteHeight * 3);
		memset(alphaData, 0, spriteWidth * spriteHeight); // Fully transparent

		// Get sprite data for each part
		for (int y = 0; y < gameSprite->height; ++y) {
			for (int x = 0; x < gameSprite->width; ++x) {
				int spriteIndex = gameSprite->getIndex(x, y, 0, 0, 0, 0, 0);
				if (spriteIndex >= 0 && spriteIndex < (int)gameSprite->spriteList.size()) {
					auto* normalImage = gameSprite->spriteList[spriteIndex];
					if (normalImage) {
						uint8_t* rgbaData = normalImage->getRGBAData();
						if (rgbaData) {
							// Copy pixel data to the correct position
							int destX = (gameSprite->width - 1 - x) * 32;
							int destY = (gameSprite->height - 1 - y) * 32;

							for (int py = 0; py < 32; ++py) {
								for (int px = 0; px < 32; ++px) {
									int srcIdx = (py * 32 + px) * 4;
									int destIdx = (destY + py) * spriteWidth + (destX + px);

									// Only copy non-transparent pixels
									uint8_t alpha = rgbaData[srcIdx + 3];
									if (alpha > 0) {
										imgData[destIdx * 3 + 0] = rgbaData[srcIdx + 0]; // R
										imgData[destIdx * 3 + 1] = rgbaData[srcIdx + 1]; // G
										imgData[destIdx * 3 + 2] = rgbaData[srcIdx + 2]; // B
										alphaData[destIdx] = alpha;
									}
								}
							}
						}
					}
				}
			}
		}
		return;
	}

	// Fallback: use DC-based rendering
	wxBitmap bmp(32, 32, 32);
	wxMemoryDC dc(bmp);
	dc.SetBackground(*wxWHITE_BRUSH);
	dc.Clear();
	sprite->DrawTo(&dc, SPRITE_SIZE_32x32, 0, 0, 32, 32);
	dc.SelectObject(wxNullBitmap);
	image = bmp.ConvertToImage();
}

int LuaImage::getWidth() const {
	return image.IsOk() ? image.GetWidth() : 0;
}

int LuaImage::getHeight() const {
	return image.IsOk() ? image.GetHeight() : 0;
}

bool LuaImage::isValid() const {
	return image.IsOk();
}

LuaImage LuaImage::resize(int width, int height, bool smooth) const {
	LuaImage result;
	if (image.IsOk() && width > 0 && height > 0) {
		wxImageResizeQuality quality = smooth ? wxIMAGE_QUALITY_HIGH : wxIMAGE_QUALITY_NEAREST;
		result.image = image.Scale(width, height, quality);
		result.filePath = filePath;
		result.spriteId = spriteId;
		result.spriteSource = spriteSource;
	}
	return result;
}

LuaImage LuaImage::scale(double factor, bool smooth) const {
	if (factor <= 0 || !image.IsOk()) {
		return LuaImage();
	}
	int newWidth = static_cast<int>(image.GetWidth() * factor);
	int newHeight = static_cast<int>(image.GetHeight() * factor);
	return resize(newWidth, newHeight, smooth);
}

wxBitmap LuaImage::getBitmap() const {
	if (!image.IsOk()) {
		return wxNullBitmap;
	}
	return wxBitmap(image);
}

wxBitmap LuaImage::getBitmap(int width, int height, bool smooth) const {
	if (!image.IsOk() || width <= 0 || height <= 0) {
		return wxNullBitmap;
	}
	wxImageResizeQuality quality = smooth ? wxIMAGE_QUALITY_HIGH : wxIMAGE_QUALITY_NEAREST;
	wxImage scaled = image.Scale(width, height, quality);
	return wxBitmap(scaled);
}

bool LuaImage::operator==(const LuaImage& other) const {
	// Compare by path if file-based, or sprite ID if sprite-based
	if (spriteSource && other.spriteSource) {
		return spriteId == other.spriteId;
	}
	if (!spriteSource && !other.spriteSource) {
		return filePath == other.filePath;
	}
	return false;
}

void registerImage(sol::state& lua) {
	// Register LuaImage as "Image" usertype
	lua.new_usertype<LuaImage>("Image",
		// Constructors
		sol::constructors<
			LuaImage(),
			LuaImage(const std::string&)
		>(),

		// Alternative constructor from table: Image{path = "..."} or Image{itemid = 100}
		sol::call_constructor, sol::factories(
			// Default constructor
			[]() { return LuaImage(); },
			// Path constructor
			[](const std::string& path) { return LuaImage(path); },
			// Table constructor
			[](sol::table t) {
				if (t["path"].valid()) {
					return LuaImage::loadFromFile(t.get<std::string>("path"));
				}
				if (t["itemid"].valid()) {
					return LuaImage::loadFromItemSprite(t.get<int>("itemid"));
				}
				if (t["spriteid"].valid()) {
					return LuaImage::loadFromSprite(t.get<int>("spriteid"));
				}
				return LuaImage();
			}
		),

		// Static factory methods
		"fromFile", &LuaImage::loadFromFile,
		"fromItemSprite", &LuaImage::loadFromItemSprite,
		"fromSprite", &LuaImage::loadFromSprite,

		// Properties (read-only)
		"width", sol::property(&LuaImage::getWidth),
		"height", sol::property(&LuaImage::getHeight),
		"valid", sol::property(&LuaImage::isValid),
		"path", sol::property(&LuaImage::getPath),
		"spriteId", sol::property(&LuaImage::getSpriteId),
		"isFromSprite", sol::property(&LuaImage::isSpriteSource),

		// Methods
		"resize", [](const LuaImage& img, int w, int h, sol::optional<bool> smooth) {
			return img.resize(w, h, smooth.value_or(true));
		},
		"scale", [](const LuaImage& img, double factor, sol::optional<bool> smooth) {
			return img.scale(factor, smooth.value_or(true));
		},

		// Equality comparison
		sol::meta_function::equal_to, &LuaImage::operator==,

		// String representation
		sol::meta_function::to_string, [](const LuaImage& img) {
			if (!img.isValid()) {
				return std::string("Image(invalid)");
			}
			if (img.isSpriteSource()) {
				return "Image(sprite=" + std::to_string(img.getSpriteId()) +
					   ", " + std::to_string(img.getWidth()) + "x" + std::to_string(img.getHeight()) + ")";
			}
			return "Image(\"" + img.getPath() + "\", " +
				   std::to_string(img.getWidth()) + "x" + std::to_string(img.getHeight()) + ")";
		}
	);
}

} // namespace LuaAPI
