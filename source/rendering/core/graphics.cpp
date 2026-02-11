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

#include "game/sprites.h"
#include "rendering/core/graphics.h"
#include <nanovg.h>
#include <spdlog/spdlog.h>
#include <nanovg_gl.h>
#include "io/filehandle.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "map/otml.h"
#include "rendering/io/editor_sprite_loader.h"

#include <wx/mstream.h>
#include <wx/dir.h>
#include "rendering/utilities/wx_utils.h"

#include "rendering/core/outfit_colors.h"
#include "rendering/core/outfit_colorizer.h"
#include <atomic>
#include <functional>

GraphicManager::GraphicManager() :
	client_version(nullptr),
	unloaded(true),
	dat_format(DAT_FORMAT_UNKNOWN),
	otfi_found(false),
	is_extended(false),
	has_transparency(false),
	has_frame_durations(false),
	has_frame_groups(false) {
	animation_timer = std::make_unique<RenderTimer>();
	animation_timer->Start();
}

GraphicManager::~GraphicManager() {
	// Unique pointers handle deletion automatically
	// atlas_manager_ clean up still good to be explicit if it has custom clear logic
	if (atlas_manager_) {
		atlas_manager_->clear();
	}
}

bool GraphicManager::hasTransparency() const {
	return has_transparency;
}

bool GraphicManager::isUnloaded() const {
	return unloaded;
}

void GraphicManager::updateTime() {
	cached_time_ = time(nullptr);
}

void GraphicManager::clear() {
	sprite_space.clear();
	image_space.clear();
	// editor_sprite_space.clear(); // Editor sprites are global/internal and should persist across version changes
	resident_images.clear();
	resident_game_sprites.clear();

	item_count = 0;
	creature_count = 0;
	collector.Clear();
	spritefile = "";

	// Cleanup atlas manager (will be reinitialized lazily when needed)
	if (atlas_manager_) {
		atlas_manager_->clear();
		atlas_manager_.reset();
	}

	unloaded = true;
}

void GraphicManager::cleanSoftwareSprites() {
	collector.CleanSoftwareSprites(sprite_space);
}

bool GraphicManager::ensureAtlasManager() {
	// Already initialized
	if (atlas_manager_ && atlas_manager_->isValid()) {
		return true;
	}

	// Create and initialize on first use
	if (!atlas_manager_) {
		atlas_manager_ = std::make_unique<AtlasManager>();
	}

	// Lazy initialization happens inside AtlasManager::ensureInitialized()
	if (!atlas_manager_->ensureInitialized()) {
		spdlog::error("GraphicManager: Failed to initialize atlas manager");
		atlas_manager_.reset();
		return false;
	}

	return true;
}

Sprite* GraphicManager::getSprite(int id) {
	if (id < 0) {
		auto it = editor_sprite_space.find(id);
		if (it != editor_sprite_space.end()) {
			return it->second.get();
		}
		return nullptr;
	}
	if (static_cast<size_t>(id) >= sprite_space.size()) {
		return nullptr;
	}
	return sprite_space[id].get();
}

void GraphicManager::insertSprite(int id, std::unique_ptr<Sprite> sprite) {
	if (id < 0) {
		editor_sprite_space[id] = std::move(sprite);
	} else {
		if (static_cast<size_t>(id) >= sprite_space.size()) {
			sprite_space.resize(id + 1);
		}
		sprite_space[id] = std::move(sprite);
	}
}

GameSprite* GraphicManager::getCreatureSprite(int id) {
	if (id < 0) {
		return nullptr;
	}

	size_t target_id = static_cast<size_t>(id) + item_count;
	if (target_id >= sprite_space.size()) {
		return nullptr;
	}
	return static_cast<GameSprite*>(sprite_space[target_id].get());
}

uint16_t GraphicManager::getItemSpriteMaxID() const {
	return item_count;
}

uint16_t GraphicManager::getCreatureSpriteMaxID() const {
	return creature_count;
}

#include "rendering/io/game_sprite_loader.h"

bool GraphicManager::loadEditorSprites() {
	return EditorSpriteLoader::Load(this);
}

bool GraphicManager::loadOTFI(const FileName& filename, wxString& error, std::vector<std::string>& warnings) {
	return GameSpriteLoader::LoadOTFI(this, filename, error, warnings);
}

bool GraphicManager::loadSpriteMetadata(const FileName& datafile, wxString& error, std::vector<std::string>& warnings) {
	return GameSpriteLoader::LoadSpriteMetadata(this, datafile, error, warnings);
}

bool GraphicManager::loadSpriteData(const FileName& datafile, wxString& error, std::vector<std::string>& warnings) {
	return GameSpriteLoader::LoadSpriteData(this, datafile, error, warnings);
}

bool GraphicManager::loadSpriteDump(std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id) {
	// Pass via reference to unique_ptr to loader
	return GameSpriteLoader::LoadSpriteDump(this, target, size, sprite_id);
}

void GraphicManager::addSpriteToCleanup(GameSprite* spr) {
	collector.AddSpriteToCleanup(spr);
}

void GraphicManager::garbageCollection() {
	collector.GarbageCollect(resident_game_sprites, resident_images, cached_time_);
}

void NVGDeleter::operator()(NVGcontext* nvg) const {
	if (nvg) {
		nvgDeleteGL3(nvg);
	}
}
