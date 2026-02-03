#include "io/loaders/spr_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "app/settings.h"
#include <vector>
#include <format>
#include <memory>

bool SprLoader::LoadData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	FileReadHandle fh(nstr(datafile.GetFullPath()));

	if (!fh.isOk()) {
#if __cpp_lib_format >= 201907L
		error = wxstr(std::format("Failed to open file {} for reading", datafile.GetFullPath().ToStdString()));
#else
		error = "Failed to open file for reading";
#endif
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

#undef safe_get

	manager->spritefile = nstr(datafile.GetFullPath());
	manager->unloaded = false;

	if (!g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
		return true;
	}

	std::vector<uint32_t> sprite_indexes = ReadSpriteIndexes(fh, total_pics, error);
	if (sprite_indexes.empty() && total_pics > 0) {
		return false;
	}

	return ReadSprites(manager, fh, sprite_indexes, warnings, error);
}

std::vector<uint32_t> SprLoader::ReadSpriteIndexes(FileReadHandle& fh, uint32_t total_pics, wxString& error) {
	std::vector<uint32_t> sprite_indexes;
	sprite_indexes.reserve(total_pics);

	for (uint32_t i = 0; i < total_pics; ++i) {
		uint32_t index;
		if (!fh.getU32(index)) {
			error = wxstr(fh.getErrorMessage());
			return {};
		}
		sprite_indexes.push_back(index);
	}
	return sprite_indexes;
}

bool SprLoader::ReadSprites(GraphicManager* manager, FileReadHandle& fh, const std::vector<uint32_t>& sprite_indexes, wxArrayString& warnings, wxString& error) {
	int id = 1;
	for (uint32_t index : sprite_indexes) {
		uint32_t seek_pos = index + 3;
		if (!fh.seek(seek_pos)) {
			// Seek failed, likely bad index or EOF?
			continue;
		}

		uint16_t size;
		if (!fh.getU16(size)) {
			error = wxstr(fh.getErrorMessage());
			return false;
		}

		if (auto it = manager->image_space.find(id); it != manager->image_space.end()) {
			GameSprite::NormalImage* spr = dynamic_cast<GameSprite::NormalImage*>(it->second.get());
			if (spr && size > 0) {
				if (spr->size > 0) {
					// Duplicate GameSprite id
#if __cpp_lib_format >= 201907L
					warnings.push_back(wxstr(std::format("items.spr: Duplicate GameSprite id {}", id)));
#else
					wxString ss;
					ss << "items.spr: Duplicate GameSprite id " << id;
					warnings.push_back(ss);
#endif
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
			target = std::make_unique<uint8_t[]>(size = sprite_size);
			if (fh.getRAW(target.get(), sprite_size)) {
				return true;
			}
			target.reset();
		}
	}
	return false;
}
