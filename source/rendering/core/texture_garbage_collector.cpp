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

#include "app/main.h"
#include "rendering/core/texture_garbage_collector.h"
#include "rendering/core/graphics.h"
#include "app/settings.h"
#include <algorithm>

TextureGarbageCollector::TextureGarbageCollector() :
	loaded_textures(0),
	lastclean(0) {
}

TextureGarbageCollector::~TextureGarbageCollector() {
}

void TextureGarbageCollector::Clear() {
	loaded_textures = 0;
	lastclean = time(nullptr);
	cleanup_list.clear();
}

void TextureGarbageCollector::NotifyTextureLoaded() {
	loaded_textures++;
}

void TextureGarbageCollector::NotifyTextureUnloaded() {
	loaded_textures--;
}

void TextureGarbageCollector::AddSpriteToCleanup(GameSprite* spr) {
	cleanup_list.push_back(spr);
	// Clean if needed
	if (cleanup_list.size() > std::max<uint32_t>(100, g_settings.getInteger(Config::SOFTWARE_CLEAN_THRESHOLD))) {
		for (int i = 0; i < g_settings.getInteger(Config::SOFTWARE_CLEAN_SIZE) && static_cast<uint32_t>(i) < cleanup_list.size(); ++i) {
			cleanup_list.front()->unloadDC();
			cleanup_list.pop_front();
		}
	}
}

void TextureGarbageCollector::GarbageCollect(std::map<int, Sprite*>& sprite_space, std::map<int, void*>& image_space) {
	if (g_settings.getInteger(Config::TEXTURE_MANAGEMENT)) {
		time_t t = time(nullptr);
		if (loaded_textures > g_settings.getInteger(Config::TEXTURE_CLEAN_THRESHOLD) && t - lastclean > g_settings.getInteger(Config::TEXTURE_CLEAN_PULSE)) {
			// We cast void* back to Image* here. This is ugly but avoids circular dependency hell for now.
			// Ideally Image should be in its own header.
			// But GameSprite::Image is a nested class.
			// We can use a template or just assume the caller handles this iteration?
			// Actually the iteration logic IS the garbage collection.
			// Let's rely on GraphicManager to pass the iterators or just handle the logic here assuming we know the types.
			// Since we include graphics.h, we know GameSprite::Image.

			// Re-implementing loops:
			for (auto& pair : image_space) {
				GameSprite::Image* img = static_cast<GameSprite::Image*>(pair.second);
				img->clean(t);
			}

			for (auto& pair : sprite_space) {
				GameSprite* gs = dynamic_cast<GameSprite*>(pair.second);
				if (gs) {
					gs->clean(t);
				}
			}
			lastclean = t;
		}
	}
}

void TextureGarbageCollector::CleanSoftwareSprites(std::map<int, Sprite*>& sprite_space) {
	for (auto& pair : sprite_space) {
		if (pair.first >= 0) { // Don't clean internal sprites
			pair.second->unloadDC();
		}
	}
}
