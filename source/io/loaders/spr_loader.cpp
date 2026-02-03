#include "io/loaders/spr_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "app/settings.h"
#include <vector>
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
	#include <format>
#endif
#include <memory>

// Anonymous namespace for constants
namespace {
	constexpr uint32_t SPRITE_DATA_OFFSET = 3;
	constexpr uint32_t SPRITE_ADDRESS_SIZE_EXTENDED = 4;
	constexpr uint32_t SPRITE_ADDRESS_SIZE_NORMAL = 2;
}

bool SprLoader::LoadData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	FileReadHandle fh(nstr(datafile.GetFullPath()));

	if (!fh.isOk()) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
		error = wxstr(std::format("Failed to open file {} for reading", datafile.GetFullPath().ToStdString()));
#else
		error = "Failed to open file for reading";
#endif
		return false;
	}

	// Local helper lambda for safe get
	auto safe_get_u32 = [&](uint32_t& out) -> bool {
		if (!fh.getU32(out)) {
			error = wxstr(fh.getErrorMessage());
			return false;
		}
		return true;
	};
	auto safe_get_u16 = [&](uint16_t& out) -> bool {
		if (!fh.getU16(out)) {
			error = wxstr(fh.getErrorMessage());
			return false;
		}
		return true;
	};

	uint32_t sprSignature;
	if (!safe_get_u32(sprSignature)) {
		return false;
	}

	uint32_t total_pics = 0;
	if (manager->is_extended) {
		if (!safe_get_u32(total_pics)) {
			return false;
		}
	} else {
		uint16_t u16 = 0;
		if (!safe_get_u16(u16)) {
			return false;
		}
		total_pics = u16;
	}

	if (total_pics > MAX_SPRITES) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
		error = wxstr(std::format("Sprite count {} exceeds limit (MAX_SPRITES={})", total_pics, MAX_SPRITES));
#else
		wxString err;
		err.Printf("Sprite count %u exceeds limit (MAX_SPRITES=%u)", total_pics, MAX_SPRITES);
		error = err;
#endif
		return false;
	}

	manager->spritefile = nstr(datafile.GetFullPath());
	manager->unloaded = false;

	if (!g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
		return true;
	}

	// Pre-allocate image_space if total_pics is known
	// Resize image_space to match exact sprite count, removing potential stale entries
	manager->image_space.resize(total_pics + 1);

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
		if (index == 0) {
			id++;
			continue;
		}

		uint32_t seek_pos = index + SPRITE_DATA_OFFSET;
		if (!fh.seek(seek_pos)) {
			// Seek failed, likely bad index or EOF. Log it.
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
			warnings.push_back(wxstr(std::format("SprLoader: Failed to seek to sprite data at offset {} for id {}", seek_pos, id)));
#else
			wxString ss;
			ss << "SprLoader: Failed to seek to sprite data at offset " << seek_pos << " for id " << id;
			warnings.push_back(ss);
#endif
			continue;
		}

		uint16_t size;
		if (!fh.getU16(size)) {
			error = wxstr(fh.getErrorMessage());
			return false;
		}

		if (id < manager->image_space.size() && manager->image_space[id]) {
			GameSprite::NormalImage* spr = dynamic_cast<GameSprite::NormalImage*>(manager->image_space[id].get());
			if (spr) {
				if (size > 0) {
					if (spr->size > 0) {
						// Duplicate GameSprite id
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
						warnings.push_back(wxstr(std::format("items.spr: Duplicate GameSprite id {}", id)));
#else
						wxString ss;
						ss << "items.spr: Duplicate GameSprite id " << id;
						warnings.push_back(ss);
#endif
						if (!fh.seekRelative(size)) {
							error = wxstr(fh.getErrorMessage());
							return false;
						}
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
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
				warnings.push_back(wxstr(std::format("SprLoader: Failed to cast sprite id {} to NormalImage", id)));
#else
				wxString ss;
				ss.Printf("SprLoader: Failed to cast sprite id %d to NormalImage", id);
				warnings.push_back(ss);
#endif
				if (size > 0) {
					if (!fh.seekRelative(size)) {
						error = wxstr(fh.getErrorMessage());
						return false;
					}
				}
			}
		} else {
			if (!fh.seekRelative(size)) {
				error = wxstr(fh.getErrorMessage());
				return false;
			}
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

	if (manager->spritefile.empty()) {
		return false;
	}

	FileReadHandle fh(manager->spritefile);
	if (!fh.isOk()) {
		return false;
	}
	manager->unloaded = false;

	uint32_t address_size = manager->is_extended ? SPRITE_ADDRESS_SIZE_EXTENDED : SPRITE_ADDRESS_SIZE_NORMAL;
	if (!fh.seek(address_size + sprite_id * sizeof(uint32_t))) {
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
		if (!fh.seek(to_seek + SPRITE_DATA_OFFSET)) {
			return false;
		}
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
