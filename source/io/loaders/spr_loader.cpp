#include "io/loaders/spr_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "app/settings.h"
#include <vector>

bool SprLoader::LoadData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	FileReadHandle fh(nstr(datafile.GetFullPath()));

	if (!fh.isOk()) {
		error = "Failed to open file for reading";
		return false;
	}

#define safe_get(func, ...)                      \
	do {                                         \
		if (!fh.get##func(__VA_ARGS__)) {        \
			error = wxstr(fh.getErrorMessage()); \
			return false;                        \
		}                                        \
	} while (false)

	uint32_t sprSignature;
	safe_get(U32, sprSignature);

	uint32_t total_pics = 0;
	if (manager->is_extended) {
		safe_get(U32, total_pics);
	} else {
		uint16_t u16 = 0;
		safe_get(U16, u16);
		total_pics = u16;
	}

	manager->spritefile = nstr(datafile.GetFullPath());
	manager->unloaded = false;

	if (!g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
		return true;
	}

	std::vector<uint32_t> sprite_indexes;
	sprite_indexes.reserve(total_pics); // Optimization: Reserve
	for (uint32_t i = 0; i < total_pics; ++i) {
		uint32_t index;
		safe_get(U32, index);
		sprite_indexes.push_back(index);
	}

	// Now read individual sprites
	int id = 1;
	for (uint32_t index : sprite_indexes) {
		// uint32_t index = *sprite_iter + 3; // Original logic was iterating + 3?
		// Wait, original code:
		/*
		for (std::vector<uint32_t>::iterator sprite_iter = sprite_indexes.begin(); sprite_iter != sprite_indexes.end(); ++sprite_iter, ++id) {
			uint32_t index = *sprite_iter + 3;
			fh.seek(index);
		*/
		// The index from file is likely an offset. +3 might be header offset or something.

		uint32_t seek_pos = index + 3;
		fh.seek(seek_pos);
		uint16_t size;
		safe_get(U16, size);

		if (auto it = manager->image_space.find(id); it != manager->image_space.end()) {
			GameSprite::NormalImage* spr = dynamic_cast<GameSprite::NormalImage*>(it->second.get());
			if (spr && size > 0) {
				if (spr->size > 0) {
					wxString ss;
					ss << "items.spr: Duplicate GameSprite id " << id;
					warnings.push_back(ss);
					fh.seekRelative(size);
				} else {
					spr->id = id;
					spr->size = size;
					spr->dump = std::make_unique<uint8_t[]>(size);
					if (!fh.getRAW(spr->dump.get(), size)) {
						error = wxstr(fh.getErrorMessage());
						return false;
					}
				}
			}
		} else {
			fh.seekRelative(size);
		}
		id++;
	}
#undef safe_get
	manager->unloaded = false;
	return true;
}

bool SprLoader::LoadDump(GraphicManager* manager, std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id) {
	if (sprite_id == 0) {
		// Empty GameSprite
		size = 0;
		target.reset();
		return true;
	}

	if (g_settings.getInteger(Config::USE_MEMCACHED_SPRITES) && manager->spritefile.empty()) {
		return false;
	}

	FileReadHandle fh(manager->spritefile);
	if (!fh.isOk()) {
		return false;
	}
	manager->unloaded = false;

	if (!fh.seek((manager->is_extended ? 4 : 2) + sprite_id * sizeof(uint32_t))) {
		return false;
	}

	uint32_t to_seek = 0;
	if (fh.getU32(to_seek)) {
		if (to_seek == 0) {
			// Offset 0 means empty/transparent sprite
			size = 0;
			target.reset(); // ensure null
			return true;
		}
		fh.seek(to_seek + 3);
		uint16_t sprite_size;
		if (fh.getU16(sprite_size)) {
			target = std::make_unique<uint8_t[]>(sprite_size);
			if (fh.getRAW(target.get(), sprite_size)) {
				size = sprite_size;
				return true;
			}
			target.reset();
		}
	}
	return false;
}
