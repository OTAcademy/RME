#include "io/loaders/dat_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "util/common.h"
#include <memory>
#include <format>

// Anonymous namespace for internal helpers and C++20/23 features
namespace {

	constexpr uint8_t RemapFlag(uint8_t flag, DatFormat format) {
		if (format >= DAT_FORMAT_1010) {
			if (flag == 16) {
				return DatFlagNoMoveAnimation;
			}
			if (flag > 16) {
				return flag - 1;
			}
		} else if (format >= DAT_FORMAT_86) {
			// No changes
		} else if (format >= DAT_FORMAT_78) {
			if (flag == 8) {
				return DatFlagChargeable;
			}
			if (flag > 8) {
				return flag - 1;
			}
		} else if (format >= DAT_FORMAT_755) {
			if (flag == 23) {
				return DatFlagFloorChange;
			}
		} else if (format >= DAT_FORMAT_74) {
			if (flag > 0 && flag <= 15) {
				return flag + 1;
			}
			switch (flag) {
				case 16:
					return DatFlagLight;
				case 17:
					return DatFlagFloorChange;
				case 18:
					return DatFlagFullGround;
				case 19:
					return DatFlagElevation;
				case 20:
					return DatFlagDisplacement;
				case 22:
					return DatFlagMinimapColor;
				case 23:
					return DatFlagRotateable;
				case 24:
					return DatFlagLyingCorpse;
				case 25:
					return DatFlagHangable;
				case 26:
					return DatFlagHookSouth;
				case 27:
					return DatFlagHookEast;
				case 28:
					return DatFlagAnimateAlways;
				case DatFlagMultiUse:
					return DatFlagForceUse;
				case DatFlagForceUse:
					return DatFlagMultiUse;
			}
		}
		return flag;
	}

	// Helper to read flag data associated with a specific flag
	void ReadFlagData(DatFormat format, FileReadHandle& file, GameSprite* sType, uint8_t flag, uint8_t previous_flag, wxArrayString& warnings) {
		switch (flag) {
			case DatFlagGroundBorder:
			case DatFlagOnBottom:
			case DatFlagOnTop:
			case DatFlagContainer:
			case DatFlagStackable:
			case DatFlagForceUse:
			case DatFlagMultiUse:
			case DatFlagFluidContainer:
			case DatFlagSplash:
			case DatFlagNotWalkable:
			case DatFlagNotMoveable:
			case DatFlagBlockProjectile:
			case DatFlagNotPathable:
			case DatFlagPickupable:
			case DatFlagHangable:
			case DatFlagHookSouth:
			case DatFlagHookEast:
			case DatFlagRotateable:
			case DatFlagDontHide:
			case DatFlagTranslucent:
			case DatFlagLyingCorpse:
			case DatFlagAnimateAlways:
			case DatFlagFullGround:
			case DatFlagLook:
			case DatFlagWrappable:
			case DatFlagUnwrappable:
			case DatFlagTopEffect:
			case DatFlagFloorChange:
			case DatFlagNoMoveAnimation:
			case DatFlagChargeable:
				break;

			case DatFlagGround:
			case DatFlagWritable:
			case DatFlagWritableOnce:
			case DatFlagCloth:
			case DatFlagLensHelp:
			case DatFlagUsable:
				file.skip(2);
				break;

			case DatFlagLight: {
				uint16_t intensity;
				uint16_t color;
				file.getU16(intensity);
				file.getU16(color);
				sType->has_light = true;
				sType->light = SpriteLight { static_cast<uint8_t>(intensity), static_cast<uint8_t>(color) };
				break;
			}

			case DatFlagDisplacement: {
				if (format >= DAT_FORMAT_755) {
					uint16_t offset_x;
					uint16_t offset_y;
					file.getU16(offset_x);
					file.getU16(offset_y);

					sType->drawoffset_x = offset_x;
					sType->drawoffset_y = offset_y;
				} else {
					sType->drawoffset_x = 8;
					sType->drawoffset_y = 8;
				}
				break;
			}

			case DatFlagElevation: {
				uint16_t draw_height;
				file.getU16(draw_height);
				sType->draw_height = draw_height;
				break;
			}

			case DatFlagMinimapColor: {
				uint16_t minimap_color;
				file.getU16(minimap_color);
				sType->minimap_color = minimap_color;
				break;
			}

			case DatFlagMarket: {
				file.skip(6);
				std::string marketName;
				file.getString(marketName);
				file.skip(4);
				break;
			}

			default: {
#if __cpp_lib_format >= 201907L
				std::string err = std::format("Metadata: Unknown flag: {}. Previous flag: {}.", static_cast<int>(flag), static_cast<int>(previous_flag));
				warnings.push_back(err);
#else
				wxString err;
				err.Printf("Metadata: Unknown flag: %d. Previous flag: %d.", static_cast<int>(flag), static_cast<int>(previous_flag));
				warnings.push_back(err);
#endif
				break;
			}
		}
	}

	// Helper loop to read all flags for a sprite
	void LoadMetadataFlags(DatFormat format, FileReadHandle& file, GameSprite* sType, uint32_t sprite_id, wxArrayString& warnings) {
		uint8_t flag = DatFlagLast;
		uint8_t previous_flag = DatFlagLast;
		for (int i = 0; i < DatFlagLast; ++i) {
			previous_flag = flag;
			file.getU8(flag);
			if (flag == DatFlagLast) {
				return;
			}
			flag = RemapFlag(flag, format);
			ReadFlagData(format, file, sType, flag, previous_flag, warnings);
		}
		// Sanity check: If we exit the loop without hitting DatFlagLast, it's potential corruption.
#if __cpp_lib_format >= 201907L
		warnings.push_back(std::format("Metadata: corruption warning - flag list exceeded limit (255) without terminator for sprite id {}", sprite_id));
#else
		wxString err;
		err.Printf("Metadata: corruption warning - flag list exceeded limit (255) without terminator for sprite id %d", sprite_id);
		warnings.push_back(err);
#endif
	}

} // namespace

bool DatLoader::LoadMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	// items.otb has most of the info we need. This only loads the GameSprite metadata
	FileReadHandle file(nstr(datafile.GetFullPath()));

	if (!file.isOk()) {
#if __cpp_lib_format >= 201907L
		// C++20 std::format
		error += std::format("Failed to open {} for reading\nThe error reported was: {}", datafile.GetFullPath().ToStdString(), file.getErrorMessage());
#else
		error += "Failed to open " + datafile.GetFullPath() + " for reading\nThe error reported was:" + wxstr(file.getErrorMessage());
#endif
		return false;
	}

	uint16_t effect_count, distance_count;

	uint32_t datSignature;
	file.getU32(datSignature);
	// get max id
	file.getU16(manager->item_count);
	file.getU16(manager->creature_count);
	file.getU16(effect_count);
	file.getU16(distance_count);

	constexpr uint32_t minID = 100; // items start with id 100
	const uint32_t maxID = manager->item_count + manager->creature_count;

	manager->dat_format = manager->client_version->getDatFormatForSignature(datSignature);

	if (!manager->otfi_found) {
		manager->is_extended = manager->dat_format >= DAT_FORMAT_96;
		manager->has_frame_durations = manager->dat_format >= DAT_FORMAT_1050;
		manager->has_frame_groups = manager->dat_format >= DAT_FORMAT_1057;
	}

	uint32_t id = minID;
	while (id <= maxID) {
		auto sTypeUnique = std::make_unique<GameSprite>();
		GameSprite* sType = sTypeUnique.get();
		manager->sprite_space[id] = std::move(sTypeUnique);

		sType->id = id;

		// Load flags
		LoadMetadataFlags(manager->dat_format, file, sType, id, warnings);

		// Reads the group count
		uint8_t group_count = 1;
		if (manager->has_frame_groups && id > manager->item_count) {
			file.getU8(group_count);
		}

		for (uint32_t k = 0; k < group_count; ++k) {
			ReadSpriteGroup(manager, file, sType, k, warnings);
		}
		++id;
	}

	return true;
}

void DatLoader::ReadSpriteGroup(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, uint32_t group_index, wxArrayString& warnings) {

	// Skipping the group type
	if (manager->has_frame_groups && sType->id > manager->item_count) {
		file.skip(1);
	}

	uint8_t width, height, layers, pattern_x, pattern_y, pattern_z, frames;

	// Size and GameSprite data
	file.getByte(width);
	file.getByte(height);

	// Skipping the exact size
	if ((width > 1) || (height > 1)) {
		file.skip(1);
	}

	file.getU8(layers);
	file.getU8(pattern_x);
	file.getU8(pattern_y);
	if (manager->dat_format <= DAT_FORMAT_74) {
		pattern_z = 1;
	} else {
		file.getU8(pattern_z);
	}
	file.getU8(frames); // Length of animation

	if (group_index == 0) {
		sType->width = width;
		sType->height = height;
		sType->layers = layers;
		sType->pattern_x = pattern_x;
		sType->pattern_y = pattern_y;
		sType->pattern_z = pattern_z;
		sType->frames = frames;
	}

	if (frames > 1) {
		uint8_t async = 0;
		int loop_count = 0;
		int8_t start_frame = 0;
		if (manager->has_frame_durations) {
			file.getByte(async);
			file.get32(loop_count);
			file.getSByte(start_frame);
		}

		if (group_index == 0) {
			sType->animator = std::make_unique<Animator>(frames, start_frame, loop_count, async == 1);
		}

		if (manager->has_frame_durations) {
			for (int i = 0; i < frames; i++) {
				uint32_t min;
				uint32_t max;
				file.getU32(min);
				file.getU32(max);
				if (group_index == 0) {
					FrameDuration* frame_duration = sType->animator->getFrameDuration(i);
					frame_duration->setValues(static_cast<int>(min), static_cast<int>(max));
				}
			}
			if (group_index == 0) {
				sType->animator->reset();
			}
		}
	}

	uint32_t numsprites = static_cast<uint32_t>(width) * static_cast<uint32_t>(height) * static_cast<uint32_t>(layers) * static_cast<uint32_t>(pattern_x) * static_cast<uint32_t>(pattern_y) * static_cast<uint32_t>(pattern_z) * static_cast<uint32_t>(frames);
	if (group_index == 0) {
		sType->numsprites = numsprites;
	}

	// Read the sprite ids
	for (uint32_t i = 0; i < numsprites; ++i) {
		uint32_t sprite_id;
		if (manager->is_extended) {
			file.getU32(sprite_id);
		} else {
			uint16_t u16 = 0;
			file.getU16(u16);
			sprite_id = u16;
		}

		if (group_index == 0) {
			auto [it, inserted] = manager->image_space.try_emplace(sprite_id, nullptr);
			if (inserted) {
				auto img = std::make_unique<GameSprite::NormalImage>();
				img->id = sprite_id;
				it->second = std::move(img);
			}
			sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(it->second.get()));
		}
	}
}
