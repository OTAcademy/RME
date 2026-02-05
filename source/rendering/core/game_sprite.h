//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_CORE_GAME_SPRITE_H_
#define RME_RENDERING_CORE_GAME_SPRITE_H_

#include "game/outfit.h"
#include "util/common.h"
#include "rendering/core/animator.h"
#include "rendering/core/sprite_light.h"
#include "rendering/core/texture_garbage_collector.h"
#include "rendering/core/atlas_manager.h"
#include "rendering/core/render_timer.h"

#include <deque>
#include <memory>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <wx/dc.h>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>

enum SpriteSize {
	SPRITE_SIZE_16x16,
	// SPRITE_SIZE_24x24,
	SPRITE_SIZE_32x32,
	SPRITE_SIZE_64x64,
	SPRITE_SIZE_COUNT
};

class GraphicManager;

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

		GameSprite* parent = nullptr;
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
			return size == rk.size && colorHash == rk.colorHash && mountColorHash == rk.mountColorHash && lookMount == rk.lookMount && lookAddon == rk.lookAddon && lookMountHead == rk.lookMountHead && lookMountBody == rk.lookMountBody && lookMountLegs == rk.lookMountLegs && lookMountFeet == rk.lookMountFeet;
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

protected:
	// Cache for default state (0,0,0,0) to avoid lookups/virtual calls for simple sprites
	mutable const AtlasRegion* cached_default_region = nullptr;
};

#endif
