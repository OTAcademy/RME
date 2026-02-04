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

#ifndef RME_GRAPHICS_H_
#define RME_GRAPHICS_H_

#include "game/outfit.h"
#include "util/common.h"
#include <deque>
#include <memory>
#include <map>

struct NVGcontext;
struct NVGDeleter {
	void operator()(NVGcontext* nvg) const;
};

#include <unordered_map>
#include <list>
#include <vector>

#include "app/client_version.h"

enum SpriteSize {
	SPRITE_SIZE_16x16,
	// SPRITE_SIZE_24x24,
	SPRITE_SIZE_32x32,
	SPRITE_SIZE_64x64,
	SPRITE_SIZE_COUNT
};

#include "animator.h"

class MapCanvas;
class GraphicManager;
class FileReadHandle;
class Animator;

#include "rendering/core/sprite_light.h"
#include "rendering/core/texture_garbage_collector.h"
#include "rendering/core/render_timer.h"
#include "rendering/core/atlas_manager.h"

class Sprite {
public:
	Sprite() { }
	virtual ~Sprite() { }

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) = 0;
	virtual void unloadDC() = 0;

private:
	Sprite(const Sprite&);
	Sprite& operator=(const Sprite&);
};

class GameSprite;
class CreatureSprite : public Sprite {
public:
	CreatureSprite(GameSprite* parent, const Outfit& outfit);
	virtual ~CreatureSprite();

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) override;
	virtual void unloadDC() override;

	GameSprite* parent;
	Outfit outfit;
};

class GameSprite : public Sprite {
public:
	GameSprite();
	~GameSprite();

	int getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const;

	// Phase 2: Get atlas region for texture array rendering
	const AtlasRegion* getAtlasRegion(int _x, int _y, int _layer, int _subtype, int _pattern_x, int _pattern_y, int _pattern_z, int _frame);
	const AtlasRegion* getAtlasRegion(int _x, int _y, int _dir, int _addon, int _pattern_z, const Outfit& _outfit, int _frame);

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) override;
	virtual void DrawTo(wxDC* dc, SpriteSize sz, const Outfit& outfit, int start_x, int start_y, int width = -1, int height = -1);

	virtual void unloadDC() override;

	void clean(time_t time);

	int getDrawHeight() const;
	std::pair<int, int> getDrawOffset() const;
	uint8_t getMiniMapColor() const;

	bool hasLight() const noexcept {
		return has_light;
	}
	const SpriteLight& getLight() const noexcept {
		return light;
	}

protected:
	class Image;
	class NormalImage;
	class TemplateImage;

	wxMemoryDC* getDC(SpriteSize size);
	wxMemoryDC* getDC(SpriteSize size, const Outfit& outfit);
	TemplateImage* getTemplateImage(int sprite_index, const Outfit& outfit);

	class Image {
	public:
		Image();
		virtual ~Image();

		bool isGLLoaded;
		time_t lastaccess;

		void visit();
		virtual void clean(time_t time);

		virtual std::unique_ptr<uint8_t[]> getRGBData() = 0;
		virtual std::unique_ptr<uint8_t[]> getRGBAData() = 0;

	protected:
		// Helper to handle atlas interactions
		const AtlasRegion* EnsureAtlasSprite(uint32_t sprite_id);
	};

	class NormalImage : public Image {
	public:
		NormalImage();
		virtual ~NormalImage();

		// We use the sprite id as key
		uint32_t id;
		const AtlasRegion* atlas_region; // AtlasRegion in texture array (nullptr if not loaded)

		// This contains the pixel data
		uint16_t size;
		std::unique_ptr<uint8_t[]> dump;

		virtual void clean(time_t time) override;

		virtual std::unique_ptr<uint8_t[]> getRGBData() override;
		virtual std::unique_ptr<uint8_t[]> getRGBAData() override;

		// Phase 2: Get atlas region (ensures loaded first)
		const AtlasRegion* getAtlasRegion();
	};

	class TemplateImage : public Image {
	public:
		TemplateImage(GameSprite* parent, int v, const Outfit& outfit);
		virtual ~TemplateImage();

		virtual void clean(time_t time) override;

		virtual std::unique_ptr<uint8_t[]> getRGBData() override;
		virtual std::unique_ptr<uint8_t[]> getRGBAData() override;

		const AtlasRegion* getAtlasRegion();
		const AtlasRegion* atlas_region;

		uint32_t texture_id; // Unique ID for AtlasManager key
		GameSprite* parent;
		int sprite_index;
		uint8_t lookHead;
		uint8_t lookBody;
		uint8_t lookLegs;
		uint8_t lookFeet;
	};

	uint32_t id;
	std::unique_ptr<wxMemoryDC> dc[SPRITE_SIZE_COUNT];
	std::unique_ptr<wxBitmap> bm[SPRITE_SIZE_COUNT];

public:
	// GameSprite info
	uint8_t height;
	uint8_t width;
	uint8_t layers;
	uint8_t pattern_x;
	uint8_t pattern_y;
	uint8_t pattern_z;
	uint8_t frames;
	uint32_t numsprites;

	std::unique_ptr<Animator> animator;

	uint16_t draw_height;
	uint16_t drawoffset_x;
	uint16_t drawoffset_y;

	uint16_t minimap_color;

	bool has_light = false;
	SpriteLight light;

	std::vector<NormalImage*> spriteList;
	std::list<std::unique_ptr<TemplateImage>> instanced_templates; // Templates that use this sprite
	struct CachedDC {
		std::unique_ptr<wxMemoryDC> dc;
		std::unique_ptr<wxBitmap> bm;
	};

	struct RenderKey {
		SpriteSize size;
		uint32_t colorHash;
		uint32_t mountColorHash;
		int lookMount, lookAddon, lookMountHead, lookMountBody, lookMountLegs, lookMountFeet;

		bool operator==(const RenderKey& rk) const {
			return size == rk.size && colorHash == rk.colorHash && mountColorHash == rk.mountColorHash && lookMount == rk.lookMount && lookAddon == rk.lookAddon && lookMountHead == rk.lookMountHead && lookMountBody == rk.lookMountBody && lookMountLegs == rk.lookMountLegs && rk.lookMountFeet == lookMountFeet;
		}
	};

	struct RenderKeyHash {
		size_t operator()(const RenderKey& k) const noexcept {
			// Combine hashes of the most significant fields
			size_t h = std::hash<uint64_t> {}((uint64_t(k.colorHash) << 32) | k.mountColorHash);
			h ^= std::hash<uint64_t> {}((uint64_t(k.lookMount) << 32) | k.lookAddon) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<uint64_t> {}((uint64_t(k.lookMountHead) << 32) | k.lookMountBody) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
	std::unordered_map<RenderKey, std::unique_ptr<CachedDC>, RenderKeyHash> colored_dc;

	bool is_resident = false; // Tracks if this GameSprite is in resident_game_sprites

	friend class GraphicManager;
	friend class GameSpriteLoader;
	friend class DatLoader;
	friend class SprLoader;
	friend class SpriteIconGenerator;
	friend class TextureGarbageCollector;
	friend class TooltipDrawer;
};

class GraphicManager {
public:
	GraphicManager();
	~GraphicManager();

	void clear();
	void cleanSoftwareSprites();

	Sprite* getSprite(int id);
	void updateTime();
	GameSprite* getCreatureSprite(int id);
	void insertSprite(int id, std::unique_ptr<Sprite> sprite);
	// Overload for compatibility with existing raw pointer calls (takes ownership)
	void insertSprite(int id, Sprite* sprite) {
		insertSprite(id, std::unique_ptr<Sprite>(sprite));
	}

	long getElapsedTime() const {
		return animation_timer->getElapsedTime();
	}

	time_t getCachedTime() const {
		return cached_time_;
	}

	uint16_t getItemSpriteMaxID() const;
	uint16_t getCreatureSpriteMaxID() const;

	// This is part of the binary
	bool loadEditorSprites();
	// Metadata should be loaded first
	// This fills the item / creature adress space

	// This fills the item / creature adress space
	bool loadOTFI(const FileName& filename, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadata(const FileName& datafile, wxString& error, wxArrayString& warnings);
	bool loadSpriteData(const FileName& datafile, wxString& error, wxArrayString& warnings);

	friend class GameSpriteLoader;
	friend class OtfiLoader;
	friend class DatLoader;
	friend class SprLoader;

	// Cleans old & unused textures according to config settings
	void garbageCollection();
	void addSpriteToCleanup(GameSprite* spr);

	wxFileName getMetadataFileName() const {
		return metadata_file;
	}
	wxFileName getSpritesFileName() const {
		return sprites_file;
	}

	bool hasTransparency() const;
	bool isUnloaded() const;

	ClientVersion* client_version;

	// Sprite Atlas (Phase 2) - manages all game sprites in a texture array
	AtlasManager* getAtlasManager() {
		return atlas_manager_.get();
	}
	bool hasAtlasManager() const {
		return atlas_manager_ != nullptr && atlas_manager_->isValid();
	}
	// Lazy initialization of atlas
	bool ensureAtlasManager();

private:
	bool unloaded;
	// This is used if memcaching is NOT on
	std::string spritefile;
	bool loadSpriteDump(std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id);

	// Atlas manager for Phase 2 texture array rendering
	std::unique_ptr<AtlasManager> atlas_manager_ = nullptr;

	// These are indexed by ID for O(1) access
	using SpriteVector = std::vector<std::unique_ptr<Sprite>>;
	SpriteVector sprite_space;
	using ImageVector = std::vector<std::unique_ptr<GameSprite::Image>>;
	ImageVector image_space;

	// Editor sprites use negative IDs, so they need a separate map
	std::unordered_map<int, std::unique_ptr<Sprite>> editor_sprite_space;

	// Active Resident Sets: Track only what's currently occupying memory/VRAM
	// This avoids O(N) scans of the entire database.
	std::vector<void*> resident_images;
	std::vector<GameSprite*> resident_game_sprites;

	DatFormat dat_format;
	uint16_t item_count;
	uint16_t creature_count;
	bool otfi_found;
	bool is_extended;
	bool has_transparency;
	bool has_frame_durations;
	bool has_frame_groups;
	wxFileName metadata_file;
	wxFileName sprites_file;

	TextureGarbageCollector collector;

	std::unique_ptr<RenderTimer> animation_timer;
	time_t cached_time_ = 0;

	friend class GameSprite::Image;
	friend class GameSprite::NormalImage;
	friend class GameSprite::TemplateImage;
};

#include "minimap_colors.h"

#endif
