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
#include "io/filehandle.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "ui/gui.h"
#include "map/otml.h"
#include "rendering/io/editor_sprite_loader.h"

#include <wx/mstream.h>
#include <wx/dir.h>
#include "ui/pngfiles.h"

#include "../../../brushes/door_normal.xpm"
#include "../../../brushes/door_normal_small.xpm"
#include "../../../brushes/door_locked.xpm"
#include "../../../brushes/door_locked_small.xpm"
#include "../../../brushes/door_magic.xpm"
#include "../../../brushes/door_magic_small.xpm"
#include "../../../brushes/door_quest.xpm"
#include "../../../brushes/door_quest_small.xpm"
#include "../../../brushes/door_normal_alt.xpm"
#include "../../../brushes/door_normal_alt_small.xpm"
#include "../../../brushes/door_archway.xpm"
#include "../../../brushes/door_archway_small.xpm"

#include "rendering/core/outfit_colors.h"
#include "rendering/core/outfit_colorizer.h"
#include "rendering/core/gl_texture_id_generator.h"
#include <spdlog/spdlog.h>

GraphicManager::GraphicManager() :
	client_version(nullptr),
	unloaded(true),
	dat_format(DAT_FORMAT_UNKNOWN),
	otfi_found(false),
	is_extended(false),
	has_transparency(false),
	has_frame_durations(false),
	has_frame_groups(false) {
	animation_timer = newd RenderTimer();
	animation_timer->Start();
}

GraphicManager::~GraphicManager() {
	for (SpriteMap::iterator iter = sprite_space.begin(); iter != sprite_space.end(); ++iter) {
		delete iter->second;
	}

	for (ImageMap::iterator iter = image_space.begin(); iter != image_space.end(); ++iter) {
		delete iter->second;
	}

	delete animation_timer;

	// Cleanup atlas manager
	if (atlas_manager_) {
		atlas_manager_->clear();
		delete atlas_manager_;
		atlas_manager_ = nullptr;
	}
}

bool GraphicManager::hasTransparency() const {
	return has_transparency;
}

bool GraphicManager::isUnloaded() const {
	return unloaded;
}

void GraphicManager::clear() {
	SpriteMap new_sprite_space;
	for (SpriteMap::iterator iter = sprite_space.begin(); iter != sprite_space.end(); ++iter) {
		if (iter->first >= 0) { // Don't clean internal sprites
			delete iter->second;
		} else {
			new_sprite_space.insert(std::make_pair(iter->first, iter->second));
		}
	}

	for (ImageMap::iterator iter = image_space.begin(); iter != image_space.end(); ++iter) {
		delete iter->second;
	}

	sprite_space.swap(new_sprite_space);
	image_space.clear();

	item_count = 0;
	creature_count = 0;
	item_count = 0;
	creature_count = 0;
	collector.Clear();
	spritefile = "";

	// Cleanup atlas manager (will be reinitialized lazily when needed)
	if (atlas_manager_) {
		atlas_manager_->clear();
		delete atlas_manager_;
		atlas_manager_ = nullptr;
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
		atlas_manager_ = newd AtlasManager();
	}

	// Lazy initialization happens inside AtlasManager::ensureInitialized()
	if (!atlas_manager_->ensureInitialized()) {
		std::cerr << "GraphicManager: Failed to initialize atlas manager" << std::endl;
		delete atlas_manager_;
		atlas_manager_ = nullptr;
		return false;
	}

	return true;
}

Sprite* GraphicManager::getSprite(int id) {
	SpriteMap::iterator it = sprite_space.find(id);
	if (it != sprite_space.end()) {
		return it->second;
	}
	return nullptr;
}

GameSprite* GraphicManager::getCreatureSprite(int id) {
	if (id < 0) {
		return nullptr;
	}

	SpriteMap::iterator it = sprite_space.find(id + item_count);
	if (it != sprite_space.end()) {
		return static_cast<GameSprite*>(it->second);
	}
	return nullptr;
}

uint16_t GraphicManager::getItemSpriteMaxID() const {
	return item_count;
}

uint16_t GraphicManager::getCreatureSpriteMaxID() const {
	return creature_count;
}

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) {
		return nullptr;
	}
	return newd wxBitmap(img, -1);
}

#include "rendering/io/game_sprite_loader.h"

bool GraphicManager::loadEditorSprites() {
	return EditorSpriteLoader::Load(this);
}

bool GraphicManager::loadOTFI(const FileName& filename, wxString& error, wxArrayString& warnings) {
	return GameSpriteLoader::LoadOTFI(this, filename, error, warnings);
}

bool GraphicManager::loadSpriteMetadata(const FileName& datafile, wxString& error, wxArrayString& warnings) {
	return GameSpriteLoader::LoadSpriteMetadata(this, datafile, error, warnings);
}

bool GraphicManager::loadSpriteData(const FileName& datafile, wxString& error, wxArrayString& warnings) {
	return GameSpriteLoader::LoadSpriteData(this, datafile, error, warnings);
}

bool GraphicManager::loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id) {
	return GameSpriteLoader::LoadSpriteDump(this, target, size, sprite_id);
}

void GraphicManager::addSpriteToCleanup(GameSprite* spr) {
	collector.AddSpriteToCleanup(spr);
}

void GraphicManager::garbageCollection() {
	std::map<int, void*> generic_image_space;
	for (auto pair : image_space) {
		generic_image_space[pair.first] = (void*)pair.second;
	}
	collector.GarbageCollect(sprite_space, generic_image_space);
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
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::~GameSprite() {
	unloadDC();
	for (std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
		delete *iter;
	}

	delete animator;
}

void GameSprite::clean(int time) {
	for (std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
		 iter != instanced_templates.end();
		 ++iter) {
		(*iter)->clean(time);
	}
}

void GameSprite::unloadDC() {
	delete dc[SPRITE_SIZE_16x16];
	delete dc[SPRITE_SIZE_32x32];
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
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

GLuint GameSprite::getHardwareID(int _x, int _y, int _layer, int _count, int _pattern_x, int _pattern_y, int _pattern_z, int _frame) {
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
	return spriteList[v]->getHardwareID();
}

const AtlasRegion* GameSprite::getAtlasRegion(int _x, int _y, int _layer, int _count, int _pattern_x, int _pattern_y, int _pattern_z, int _frame) {
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
	return spriteList[v]->getAtlasRegion();
}

GameSprite::TemplateImage* GameSprite::getTemplateImage(int sprite_index, const Outfit& outfit) {
	if (instanced_templates.empty()) {
		TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
		instanced_templates.push_back(img);
		return img;
	}
	// While this is linear lookup, it is very rare for the list to contain more than 4-8 entries, so it's faster than a hashmap anyways.
	for (std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
		TemplateImage* img = *iter;
		if (img->sprite_index == sprite_index) {
			uint32_t lookHash = img->lookHead << 24 | img->lookBody << 16 | img->lookLegs << 8 | img->lookFeet;
			if (outfit.getColorHash() == lookHash) {
				return img;
			}
		}
	}
	TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
	instanced_templates.push_back(img);
	return img;
}

GLuint GameSprite::getHardwareID(int _x, int _y, int _dir, int _addon, int _pattern_z, const Outfit& _outfit, int _frame) {
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
		return img->getHardwareID();
	}
	return spriteList[v]->getHardwareID();
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

#include "rendering/utilities/sprite_icon_generator.h"

wxMemoryDC* GameSprite::getDC(SpriteSize size) {
	ASSERT(size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_32x32);

	if (!dc[size]) {
		dc[size] = SpriteIconGenerator::Generate(this, size);
		g_gui.gfx.addSpriteToCleanup(this);
	}
	return dc[size];
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

GameSprite::Image::Image() :
	isGLLoaded(false),
	lastaccess(0) {
	////
}

GameSprite::Image::~Image() {
	unloadGLTexture(0);
}

void GameSprite::Image::createGLTexture(GLuint whatid) {
	ASSERT(!isGLLoaded);

	uint8_t* rgba = getRGBAData();
	if (!rgba) {
		// Fallback: Create a magenta texture to distinguish failure from garbage
		// Use literal 32 to ensure compilation (OT sprites are always 32x32)
		rgba = newd uint8_t[32 * 32 * 4];
		for (int i = 0; i < 32 * 32; ++i) {
			rgba[i * 4 + 0] = 0;
			rgba[i * 4 + 1] = 0;
			rgba[i * 4 + 2] = 0;
			rgba[i * 4 + 3] = 0;
		}
	}

	isGLLoaded = true;
	g_gui.gfx.collector.NotifyTextureLoaded();

	glBindTexture(GL_TEXTURE_2D, whatid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Nearest-neighbor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest-neighbor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); // GL_CLAMP_TO_EDGE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SPRITE_PIXELS, SPRITE_PIXELS, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	delete[] rgba;
#undef SPRITE_SIZE
}

void GameSprite::Image::unloadGLTexture(GLuint whatid) {
	isGLLoaded = false;
	g_gui.gfx.collector.NotifyTextureUnloaded();
	glDeleteTextures(1, &whatid);
}

void GameSprite::Image::visit() {
	lastaccess = time(nullptr);
}

void GameSprite::Image::clean(int time) {
	if (isGLLoaded && time - lastaccess > g_settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		unloadGLTexture(0);
	}
}

GameSprite::NormalImage::NormalImage() :
	id(0),
	gl_tid(0),
	atlas_region(nullptr),
	size(0),
	dump(nullptr) {
	////
}

GameSprite::NormalImage::~NormalImage() {
	delete[] dump;
}

void GameSprite::NormalImage::clean(int time) {
	Image::clean(time);
	if (time - lastaccess > 5 && !g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) { // We keep dumps around for 5 seconds.
		delete[] dump;
		dump = nullptr;
	}
}

uint8_t* GameSprite::NormalImage::getRGBData() {
	if (!dump) {
		if (g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		}

		if (!g_gui.gfx.loadSpriteDump(dump, size, id)) {
			return nullptr;
		}
	}

	const int pixels_data_size = SPRITE_PIXELS * SPRITE_PIXELS * 3;
	uint8_t* data = newd uint8_t[pixels_data_size];
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

uint8_t* GameSprite::NormalImage::getRGBAData() {
	// Robust ID 0 handling
	if (id == 0) {
		const int pixels_data_size = SPRITE_PIXELS_SIZE * 4;
		uint8_t* data = newd uint8_t[pixels_data_size];
		memset(data, 0, pixels_data_size); // Transparent
		return data;
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
	uint8_t* data = newd uint8_t[pixels_data_size];
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

GLuint GameSprite::NormalImage::getHardwareID() {
	if (!isGLLoaded) {
		if (gl_tid == 0) {
			gl_tid = GLTextureIDGenerator::GetFreeTextureID();
		}
		createGLTexture(gl_tid);
	}
	visit();
	return gl_tid;
}

void GameSprite::NormalImage::createGLTexture(GLuint ignored) {
	// Atlas-only rendering - no legacy fallback
	if (g_gui.gfx.ensureAtlasManager()) {
		AtlasManager* atlas_mgr = g_gui.gfx.getAtlasManager();
		if (!atlas_region) {
			uint8_t* rgba = getRGBAData();
			if (rgba) {
				atlas_region = atlas_mgr->addSprite(id, rgba);
				delete[] rgba;
				if (atlas_region) {
					isGLLoaded = true;
					g_gui.gfx.collector.NotifyTextureLoaded();
					return;
				}
				spdlog::warn("Atlas addSprite failed for id={}", id);
			} else {
				spdlog::warn("getRGBAData returned null for sprite id={}, dump={}, size={}", id, (dump ? "exists" : "null"), size);
			}
		} else {
			// Already in atlas
			isGLLoaded = true;
			return;
		}
	} else {
		spdlog::error("AtlasManager not available for sprite {}", id);
	}
}

void GameSprite::NormalImage::unloadGLTexture(GLuint ignored) {
	Image::unloadGLTexture(gl_tid);
}

const AtlasRegion* GameSprite::NormalImage::getAtlasRegion() {
	if (!isGLLoaded) {
		if (gl_tid == 0) {
			gl_tid = GLTextureIDGenerator::GetFreeTextureID();
		}
		createGLTexture(gl_tid);
	}
	visit();
	return atlas_region;
}

GameSprite::TemplateImage::TemplateImage(GameSprite* parent, int v, const Outfit& outfit) :
	gl_tid(0),
	parent(parent),
	sprite_index(v),
	lookHead(outfit.lookHead),
	lookBody(outfit.lookBody),
	lookLegs(outfit.lookLegs),
	lookFeet(outfit.lookFeet),
	atlas_region(nullptr) {
	////
}

GameSprite::TemplateImage::~TemplateImage() {
	////
}

uint8_t* GameSprite::TemplateImage::getRGBData() {
	uint8_t* rgbdata = parent->spriteList[sprite_index]->getRGBData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if (!rgbdata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if (!template_rgbdata) {
		delete[] rgbdata;
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
	delete[] template_rgbdata;
	return rgbdata;
}

uint8_t* GameSprite::TemplateImage::getRGBAData() {
	uint8_t* rgbadata = parent->spriteList[sprite_index]->getRGBAData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if (!rgbadata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if (!template_rgbdata) {
		delete[] rgbadata;
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
	delete[] template_rgbdata;
	return rgbadata;
}

GLuint GameSprite::TemplateImage::getHardwareID() {
	if (!isGLLoaded) {
		if (gl_tid == 0) {
			gl_tid = GLTextureIDGenerator::GetFreeTextureID();
		}
		createGLTexture(gl_tid);
		if (!isGLLoaded) {
			return 0;
		}
	}
	visit();
	return gl_tid;
}

void GameSprite::TemplateImage::createGLTexture(GLuint unused) {
	// Atlas-only rendering - use generated gl_tid as unique ID
	if (g_gui.gfx.ensureAtlasManager()) {
		AtlasManager* atlas_mgr = g_gui.gfx.getAtlasManager();
		if (!atlas_region) {
			uint8_t* rgba = getRGBAData();
			if (!rgba) {
				spdlog::warn("getRGBAData returned null for template sprite_index={} - creating fallback", sprite_index);
				rgba = newd uint8_t[32 * 32 * 4];
				memset(rgba, 0, 32 * 32 * 4);
			}

			if (rgba) {
				// Use the unique gl_tid as the sprite identifier in the atlas
				atlas_region = atlas_mgr->addSprite(gl_tid, rgba);
				delete[] rgba;
				if (atlas_region) {
					isGLLoaded = true;
					return;
				}
				spdlog::warn("Atlas addSprite failed for template sprite_index={}", sprite_index);
			}
		} else {
			// Already in atlas
			isGLLoaded = true;
			return;
		}
	} else {
		spdlog::error("AtlasManager not available for template sprite_index={}", sprite_index);
	}
}

const AtlasRegion* GameSprite::TemplateImage::getAtlasRegion() {
	if (!isGLLoaded) {
		if (gl_tid == 0) {
			gl_tid = GLTextureIDGenerator::GetFreeTextureID();
		}
		createGLTexture(gl_tid);
	}
	visit();
	return atlas_region;
}

void GameSprite::TemplateImage::unloadGLTexture(GLuint unused) {
	Image::unloadGLTexture(gl_tid);
}
