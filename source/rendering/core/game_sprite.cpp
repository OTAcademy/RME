//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/core/game_sprite.h"
#include "rendering/core/graphics.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "rendering/core/outfit_colorizer.h"
#include "rendering/core/outfit_colors.h"
#include <spdlog/spdlog.h>
#include <atomic>

static std::atomic<uint32_t> template_id_generator(0x1000000);

CreatureSprite::CreatureSprite(GameSprite* parent, const Outfit& outfit) :
	parent(parent),
	outfit(outfit) {
}

CreatureSprite::~CreatureSprite() {
}

void CreatureSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height) {
	if (parent) {
		parent->DrawTo(dc, sz, outfit, start_x, start_y, width, height);
	}
}

void CreatureSprite::unloadDC() {
	if (parent) {
		GameSprite::RenderKey key;
		key.colorHash = outfit.getColorHash();
		key.mountColorHash = outfit.getMountColorHash();
		key.lookMount = outfit.lookMount;
		key.lookAddon = outfit.lookAddon;
		key.lookMountHead = outfit.lookMountHead;
		key.lookMountBody = outfit.lookMountBody;
		key.lookMountLegs = outfit.lookMountLegs;
		key.lookMountFeet = outfit.lookMountFeet;

		key.size = SPRITE_SIZE_16x16;
		parent->colored_dc.erase(key);

		key.size = SPRITE_SIZE_32x32;
		parent->colored_dc.erase(key);
	}
}

GameSprite::GameSprite() :
	id(0),
	height(0),
	width(0),
	layers(0),
	pattern_x(0),
	pattern_y(0),
	pattern_z(0),
	frames(0),
	numsprites(0),
	animator(nullptr),
	draw_height(0),
	drawoffset_x(0),
	drawoffset_y(0),
	minimap_color(0) {
	// dc initialized to nullptr by unique_ptr default ctor
}

GameSprite::~GameSprite() {
	unloadDC();
	// instanced_templates and animator cleaned up automatically by unique_ptr
}

void GameSprite::clean(time_t time) {
	for (auto& iter : instanced_templates) {
		iter->clean(time);
	}
}

void GameSprite::unloadDC() {
	dc[SPRITE_SIZE_16x16].reset();
	dc[SPRITE_SIZE_32x32].reset();
	bm[SPRITE_SIZE_16x16].reset();
	bm[SPRITE_SIZE_32x32].reset();
	colored_dc.clear();
}

int GameSprite::getDrawHeight() const {
	return draw_height;
}

std::pair<int, int> GameSprite::getDrawOffset() const {
	return std::make_pair(drawoffset_x, drawoffset_y);
}

uint8_t GameSprite::getMiniMapColor() const {
	return minimap_color;
}

int GameSprite::getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const {
	return ((((((frame % this->frames) * this->pattern_z + pattern_z) * this->pattern_y + pattern_y) * this->pattern_x + pattern_x) * this->layers + layer) * this->height + height) * this->width + width;
}

const AtlasRegion* GameSprite::getAtlasRegion(int _x, int _y, int _layer, int _count, int _pattern_x, int _pattern_y, int _pattern_z, int _frame) {
	// Optimization for simple static sprites (1x1, 1 frame, etc.)
	// Most ground tiles fall into this category.
	if (_count == -1 && numsprites == 1 && frames == 1 && layers == 1 && width == 1 && height == 1) {
		// Also check default params
		if (_x == 0 && _y == 0 && _layer == 0 && _frame == 0 && _pattern_x == 0 && _pattern_y == 0 && _pattern_z == 0) {
			// Check cache
			// We rely on spriteList[0] being valid for simple sprites
			// Check isGLLoaded to ensure validity of cached region (it must correspond to loaded texture)
			if (cached_default_region && spriteList[0]->isGLLoaded) {
				return cached_default_region;
			}

			// Lazy set parent for cache invalidation
			spriteList[0]->parent = this;

			const AtlasRegion* r = spriteList[0]->getAtlasRegion();
			if (spriteList[0]->isGLLoaded) {
				cached_default_region = r;
			} else {
				cached_default_region = nullptr;
			}
			return r;
		}
	}

	uint32_t v;
	if (_count >= 0 && height <= 1 && width <= 1) {
		v = _count;
	} else {
		v = ((((((_frame)*pattern_y + _pattern_y) * pattern_x + _pattern_x) * layers + _layer) * height + _y) * width + _x);
	}
	if (v >= numsprites) {
		if (numsprites == 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}

	// Ensure parent is set for invalidation (even in slow path)
	if (spriteList[v]) {
		spriteList[v]->parent = this;
	}

	return spriteList[v]->getAtlasRegion();
}

GameSprite::TemplateImage* GameSprite::getTemplateImage(int sprite_index, const Outfit& outfit) {
	// While this is linear lookup, it is very rare for the list to contain more than 4-8 entries,
	// so it's faster than a hashmap anyways.
	auto it = std::find_if(instanced_templates.begin(), instanced_templates.end(), [sprite_index, &outfit](const auto& img) {
		if (img->sprite_index != sprite_index) {
			return false;
		}
		uint32_t lookHash = img->lookHead << 24 | img->lookBody << 16 | img->lookLegs << 8 | img->lookFeet;
		return outfit.getColorHash() == lookHash;
	});

	if (it != instanced_templates.end()) {
		return it->get();
	}

	auto img = std::make_unique<TemplateImage>(this, sprite_index, outfit);
	TemplateImage* ptr = img.get();
	instanced_templates.push_back(std::move(img));
	return ptr;
}

const AtlasRegion* GameSprite::getAtlasRegion(int _x, int _y, int _dir, int _addon, int _pattern_z, const Outfit& _outfit, int _frame) {
	uint32_t v = getIndex(_x, _y, 0, _dir, _addon, _pattern_z, _frame);
	if (v >= numsprites) {
		if (numsprites == 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}
	if (layers > 1) { // Template
		TemplateImage* img = getTemplateImage(v, _outfit);
		return img->getAtlasRegion();
	}
	return spriteList[v]->getAtlasRegion();
}

wxMemoryDC* GameSprite::getDC(SpriteSize size) {
	ASSERT(size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_32x32);

	if (!dc[size]) {
		wxBitmap bmp = SpriteIconGenerator::Generate(this, size);
		if (bmp.IsOk()) {
			bm[size] = std::make_unique<wxBitmap>(bmp);
			dc[size] = std::make_unique<wxMemoryDC>(*bm[size]);
		}
		g_gui.gfx.addSpriteToCleanup(this);
	}
	return dc[size].get();
}

wxMemoryDC* GameSprite::getDC(SpriteSize size, const Outfit& outfit) {
	ASSERT(size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_32x32);

	RenderKey key;
	key.size = size;
	key.colorHash = outfit.getColorHash();
	key.mountColorHash = outfit.getMountColorHash();
	key.lookMount = outfit.lookMount;
	key.lookAddon = outfit.lookAddon;
	key.lookMountHead = outfit.lookMountHead;
	key.lookMountBody = outfit.lookMountBody;
	key.lookMountLegs = outfit.lookMountLegs;
	key.lookMountFeet = outfit.lookMountFeet;

	auto it = colored_dc.find(key);
	if (it == colored_dc.end()) {
		wxBitmap bmp = SpriteIconGenerator::Generate(this, size, outfit);
		if (bmp.IsOk()) {
			auto cache = std::make_unique<CachedDC>();
			cache->bm = std::make_unique<wxBitmap>(bmp);
			cache->dc = std::make_unique<wxMemoryDC>(*cache->bm);

			auto res = colored_dc.insert(std::make_pair(key, std::move(cache)));
			g_gui.gfx.addSpriteToCleanup(this);
			return res.first->second->dc.get();
		}
		return nullptr;
	}
	return it->second->dc.get();
}

void GameSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height) {
	if (width == -1) {
		width = sz == SPRITE_SIZE_32x32 ? 32 : 16;
	}
	if (height == -1) {
		height = sz == SPRITE_SIZE_32x32 ? 32 : 16;
	}
	wxDC* sdc = getDC(sz);
	if (sdc) {
		dc->Blit(start_x, start_y, width, height, sdc, 0, 0, wxCOPY, true);
	} else {
		const wxBrush& b = dc->GetBrush();
		dc->SetBrush(*wxRED_BRUSH);
		dc->DrawRectangle(start_x, start_y, width, height);
		dc->SetBrush(b);
	}
}

void GameSprite::DrawTo(wxDC* dc, SpriteSize sz, const Outfit& outfit, int start_x, int start_y, int width, int height) {
	if (width == -1) {
		width = sz == SPRITE_SIZE_32x32 ? 32 : 16;
	}
	if (height == -1) {
		height = sz == SPRITE_SIZE_32x32 ? 32 : 16;
	}
	wxDC* sdc = getDC(sz, outfit);
	if (sdc) {
		dc->Blit(start_x, start_y, width, height, sdc, 0, 0, wxCOPY, true);
	} else {
		const wxBrush& b = dc->GetBrush();
		dc->SetBrush(*wxRED_BRUSH);
		dc->DrawRectangle(start_x, start_y, width, height);
		dc->SetBrush(b);
	}
}

GameSprite::Image::Image() :
	isGLLoaded(false),
	lastaccess(0) {
}

GameSprite::Image::~Image() {
	// Base destructor no longer needs to unload GL texture
	// as separate textures are removed.
}

void GameSprite::Image::visit() {
	lastaccess = g_gui.gfx.getCachedTime();
}

void GameSprite::Image::clean(time_t time) {
	// Legacy texture cleanup logic removed
}

const AtlasRegion* GameSprite::Image::EnsureAtlasSprite(uint32_t sprite_id) {
	if (g_gui.gfx.ensureAtlasManager()) {
		AtlasManager* atlas_mgr = g_gui.gfx.getAtlasManager();

		// 1. Check if already loaded
		const AtlasRegion* region = atlas_mgr->getRegion(sprite_id);
		if (region) {
			return region;
		}

		// 2. Load data
		auto rgba = getRGBAData();
		if (!rgba) {
			// Fallback: Create a magenta texture to distinguish failure from garbage
			// Use literal 32 to ensure compilation (OT sprites are always 32x32)
			rgba = std::make_unique<uint8_t[]>(32 * 32 * 4);
			for (int i = 0; i < 32 * 32; ++i) {
				rgba[i * 4 + 0] = 255;
				rgba[i * 4 + 1] = 0;
				rgba[i * 4 + 2] = 255;
				rgba[i * 4 + 3] = 255;
			}
			spdlog::warn("getRGBAData returned null for sprite_id={} - using fallback", sprite_id);
		}

		// 3. Add to Atlas
		if (rgba) {
			region = atlas_mgr->addSprite(sprite_id, rgba.get());

			if (region) {
				isGLLoaded = true;
				g_gui.gfx.resident_images.push_back(this); // Add to resident set
				g_gui.gfx.collector.NotifyTextureLoaded();
				return region;
			} else {
				spdlog::warn("Atlas addSprite failed for sprite_id={}", sprite_id);
			}
		}
	} else {
		spdlog::error("AtlasManager not available for sprite_id={}", sprite_id);
	}
	return nullptr;
}

GameSprite::NormalImage::NormalImage() :
	id(0),
	atlas_region(nullptr),
	size(0),
	dump(nullptr) {
}

GameSprite::NormalImage::~NormalImage() {
	// dump auto-deleted
	if (isGLLoaded) {
		if (g_gui.gfx.hasAtlasManager()) {
			g_gui.gfx.getAtlasManager()->removeSprite(id);
		}
	}
}

void GameSprite::NormalImage::clean(time_t time) {
	// Evict from atlas if expired
	if (isGLLoaded && time - lastaccess > g_settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		if (g_gui.gfx.hasAtlasManager()) {
			g_gui.gfx.getAtlasManager()->removeSprite(id);
		}
		isGLLoaded = false;
		atlas_region = nullptr;

		if (parent) {
			parent->cached_default_region = nullptr;
		}

		// resident_images removal is handled by GC loop using swap-and-pop
		g_gui.gfx.collector.NotifyTextureUnloaded();
	}

	if (time - lastaccess > 5 && !g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) { // We keep dumps around for 5 seconds.
		dump.reset();
	}
}

std::unique_ptr<uint8_t[]> GameSprite::NormalImage::getRGBData() {
	if (id == 0) {
		const int pixels_data_size = SPRITE_PIXELS * SPRITE_PIXELS * 3;
		return std::make_unique<uint8_t[]>(pixels_data_size); // Value-initialized (zeroed)
	}

	if (!dump) {
		if (g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		}

		if (!g_gui.gfx.loadSpriteDump(dump, size, id)) {
			return nullptr;
		}
	}

	const int pixels_data_size = SPRITE_PIXELS * SPRITE_PIXELS * 3;
	auto data = std::make_unique<uint8_t[]>(pixels_data_size);
	uint8_t bpp = g_gui.gfx.hasTransparency() ? 4 : 3;
	int write = 0;
	int read = 0;

	// decompress pixels
	while (read < size && write < pixels_data_size) {
		int transparent = dump[read] | dump[read + 1] << 8;
		read += 2;
		for (int i = 0; i < transparent && write < pixels_data_size; i++) {
			data[write + 0] = 0xFF; // red
			data[write + 1] = 0x00; // green
			data[write + 2] = 0xFF; // blue
			write += 3;
		}

		int colored = dump[read] | dump[read + 1] << 8;
		read += 2;
		for (int i = 0; i < colored && write < pixels_data_size; i++) {
			data[write + 0] = dump[read + 0]; // red
			data[write + 1] = dump[read + 1]; // green
			data[write + 2] = dump[read + 2]; // blue
			write += 3;
			read += bpp;
		}
	}

	// fill remaining pixels
	while (write < pixels_data_size) {
		data[write + 0] = 0xFF; // red
		data[write + 1] = 0x00; // green
		data[write + 2] = 0xFF; // blue
		write += 3;
	}
	return data;
}

std::unique_ptr<uint8_t[]> GameSprite::NormalImage::getRGBAData() {
	// Robust ID 0 handling
	if (id == 0) {
		const int pixels_data_size = SPRITE_PIXELS_SIZE * 4;
		return std::make_unique<uint8_t[]>(pixels_data_size); // Value-initialized (zeroed)
	}

	if (!dump) {
		if (g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		}

		if (!g_gui.gfx.loadSpriteDump(dump, size, id)) {
			// This is the only case where we return nullptr for non-zero ID
			// effectively warning the caller that the sprite is missing from file
			return nullptr;
		}
	}

	const int pixels_data_size = SPRITE_PIXELS_SIZE * 4;
	auto data = std::make_unique<uint8_t[]>(pixels_data_size);
	bool use_alpha = g_gui.gfx.hasTransparency();
	uint8_t bpp = use_alpha ? 4 : 3;
	int write = 0;
	int read = 0;
	bool non_zero_alpha_found = false;
	bool non_black_pixel_found = false;

	// decompress pixels
	while (read < size && write < pixels_data_size) {
		int transparent = dump[read] | dump[read + 1] << 8;

		// Integrity check for transparency run
		if (write + (transparent * 4) > pixels_data_size) {
			spdlog::warn("Sprite {}: Transparency run overrun (transparent={}, write={}, max={})", id, transparent, write, pixels_data_size);
			transparent = (pixels_data_size - write) / 4;
		}

		read += 2;
		for (int i = 0; i < transparent && write < pixels_data_size; i++) {
			data[write + 0] = 0x00; // red
			data[write + 1] = 0x00; // green
			data[write + 2] = 0x00; // blue
			data[write + 3] = 0x00; // alpha
			write += 4;
		}

		if (read >= size || write >= pixels_data_size) {
			break;
		}

		int colored = dump[read] | dump[read + 1] << 8;
		read += 2;

		// Integrity check for colored run
		if (write + (colored * 4) > pixels_data_size) {
			spdlog::warn("Sprite {}: Colored run overrun (colored={}, write={}, max={})", id, colored, write, pixels_data_size);
			colored = (pixels_data_size - write) / 4;
		}

		// Integrity check for read buffer
		if (read + (colored * bpp) > size) {
			spdlog::warn("Sprite {}: Read buffer overrun (colored={}, bpp={}, read={}, size={})", id, colored, bpp, read, size);
			// We can't easily recover here without risking reading garbage, so stop
			break;
		}

		for (int i = 0; i < colored && write < pixels_data_size; i++) {
			uint8_t r = dump[read + 0];
			uint8_t g = dump[read + 1];
			uint8_t b = dump[read + 2];
			uint8_t a = use_alpha ? dump[read + 3] : 0xFF;

			data[write + 0] = r;
			data[write + 1] = g;
			data[write + 2] = b;
			data[write + 3] = a;

			if (a > 0) {
				non_zero_alpha_found = true;
			}
			if (r > 0 || g > 0 || b > 0) {
				non_black_pixel_found = true;
			}

			write += 4;
			read += bpp;
		}
	}

	// fill remaining pixels
	while (write < pixels_data_size) {
		data[write + 0] = 0x00; // red
		data[write + 1] = 0x00; // green
		data[write + 2] = 0x00; // blue
		data[write + 3] = 0x00; // alpha
		write += 4;
	}

	// Debug logging for diagnostic - verify if we are decoding pure transparency or pure blackness
	// Only log for a few arbitrary IDs to avoid spamming, or if suspicious
	if (!non_zero_alpha_found && id > 100) {
		// This sprite is 100% invisible. This might be correct (magic fields?) but worth noting if ALL are invisible.
		static int empty_log_count = 0;
		if (empty_log_count++ < 10) {
			spdlog::info("Sprite {}: Decoded fully transparent sprite. bpp used: {}, dump size: {}", id, bpp, size);
		}
	} else if (!non_black_pixel_found && non_zero_alpha_found && id > 100) {
		// This sprite has alpha but all RGB are 0. It is a "black shadow" or "darkness".
		// If ALL sprites look like this, we have a problem.
		static int black_log_count = 0;
		if (black_log_count++ < 10) {
			spdlog::warn("Sprite {}: Decoded PURE BLACK sprite (Alpha > 0, RGB = 0). bpp used: {}, dump size: {}. Check hasTransparency() config!", id, bpp, size);
		}
	}

	return data;
}

const AtlasRegion* GameSprite::NormalImage::getAtlasRegion() {
	if (!isGLLoaded) {
		atlas_region = EnsureAtlasSprite(id);
	}
	visit();
	return atlas_region;
}

GameSprite::TemplateImage::TemplateImage(GameSprite* parent, int v, const Outfit& outfit) :
	parent(parent),
	sprite_index(v),
	lookHead(outfit.lookHead),
	lookBody(outfit.lookBody),
	lookLegs(outfit.lookLegs),
	lookFeet(outfit.lookFeet),
	atlas_region(nullptr),
	texture_id(template_id_generator.fetch_add(1)) { // Generate unique ID for Atlas
}

GameSprite::TemplateImage::~TemplateImage() {
	if (isGLLoaded) {
		if (g_gui.gfx.hasAtlasManager()) {
			g_gui.gfx.getAtlasManager()->removeSprite(texture_id);
		}
	}
}

void GameSprite::TemplateImage::clean(time_t time) {
	// Evict from atlas if expired
	if (isGLLoaded && time - lastaccess > g_settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		if (g_gui.gfx.hasAtlasManager()) {
			g_gui.gfx.getAtlasManager()->removeSprite(texture_id);
		}
		isGLLoaded = false;
		atlas_region = nullptr;
		g_gui.gfx.collector.NotifyTextureUnloaded();
	}
}

std::unique_ptr<uint8_t[]> GameSprite::TemplateImage::getRGBData() {
	auto rgbdata = parent->spriteList[sprite_index]->getRGBData();
	auto template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if (!rgbdata) {
		// template_rgbdata auto-deleted
		return nullptr;
	}
	if (!template_rgbdata) {
		// rgbdata auto-deleted
		return nullptr;
	}

	if (lookHead > TemplateOutfitLookupTableSize) {
		lookHead = 0;
	}
	if (lookBody > TemplateOutfitLookupTableSize) {
		lookBody = 0;
	}
	if (lookLegs > TemplateOutfitLookupTableSize) {
		lookLegs = 0;
	}
	if (lookFeet > TemplateOutfitLookupTableSize) {
		lookFeet = 0;
	}

	for (int y = 0; y < SPRITE_PIXELS; ++y) {
		for (int x = 0; x < SPRITE_PIXELS; ++x) {
			uint8_t& red = rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 0];
			uint8_t& green = rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 1];
			uint8_t& blue = rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 2];

			uint8_t& tred = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 0];
			uint8_t& tgreen = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 1];
			uint8_t& tblue = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 2];

			if (tred && tgreen && !tblue) { // yellow => head
				OutfitColorizer::ColorizePixel(lookHead, red, green, blue);
			} else if (tred && !tgreen && !tblue) { // red => body
				OutfitColorizer::ColorizePixel(lookBody, red, green, blue);
			} else if (!tred && tgreen && !tblue) { // green => legs
				OutfitColorizer::ColorizePixel(lookLegs, red, green, blue);
			} else if (!tred && !tgreen && tblue) { // blue => feet
				OutfitColorizer::ColorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	// template_rgbdata auto-deleted
	return rgbdata;
}

std::unique_ptr<uint8_t[]> GameSprite::TemplateImage::getRGBAData() {
	auto rgbadata = parent->spriteList[sprite_index]->getRGBAData();
	auto template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if (!rgbadata) {
		spdlog::warn("TemplateImage: Failed to load BASE sprite data for sprite_index={} (template_id={}). Parent width={}, height={}", sprite_index, texture_id, parent->width, parent->height);
		// template_rgbdata auto-deleted
		return nullptr;
	}
	if (!template_rgbdata) {
		spdlog::warn("TemplateImage: Failed to load MASK sprite data for sprite_index={} (template_id={}) (mask_index={})", sprite_index, texture_id, sprite_index + parent->height * parent->width);
		// rgbadata auto-deleted
		return nullptr;
	}

	if (lookHead > TemplateOutfitLookupTableSize) {
		lookHead = 0;
	}
	if (lookBody > TemplateOutfitLookupTableSize) {
		lookBody = 0;
	}
	if (lookLegs > TemplateOutfitLookupTableSize) {
		lookLegs = 0;
	}
	if (lookFeet > TemplateOutfitLookupTableSize) {
		lookFeet = 0;
	}

	for (int y = 0; y < SPRITE_PIXELS; ++y) {
		for (int x = 0; x < SPRITE_PIXELS; ++x) {
			uint8_t& red = rgbadata[y * SPRITE_PIXELS * 4 + x * 4 + 0];
			uint8_t& green = rgbadata[y * SPRITE_PIXELS * 4 + x * 4 + 1];
			uint8_t& blue = rgbadata[y * SPRITE_PIXELS * 4 + x * 4 + 2];

			uint8_t& tred = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 0];
			uint8_t& tgreen = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 1];
			uint8_t& tblue = template_rgbdata[y * SPRITE_PIXELS * 3 + x * 3 + 2];

			if (tred && tgreen && !tblue) { // yellow => head
				OutfitColorizer::ColorizePixel(lookHead, red, green, blue);
			} else if (tred && !tgreen && !tblue) { // red => body
				OutfitColorizer::ColorizePixel(lookBody, red, green, blue);
			} else if (!tred && tgreen && !tblue) { // green => legs
				OutfitColorizer::ColorizePixel(lookLegs, red, green, blue);
			} else if (!tred && !tgreen && tblue) { // blue => feet
				OutfitColorizer::ColorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	// template_rgbdata auto-deleted
	return rgbadata;
}

const AtlasRegion* GameSprite::TemplateImage::getAtlasRegion() {
	if (!isGLLoaded) {
		atlas_region = EnsureAtlasSprite(texture_id);
	}
	visit();
	return atlas_region;
}
