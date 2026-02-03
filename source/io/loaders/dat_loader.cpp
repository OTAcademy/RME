#include "io/loaders/dat_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "util/common.h"
#include <memory>
#include <cstdint>
#include <climits>
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
	#include <format>
#endif

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
	bool ReadFlagData(DatFormat format, FileReadHandle& file, GameSprite* sType, uint8_t flag, uint8_t previous_flag, wxArrayString& warnings) {
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
			case DatFlagDefault:
				break;

			case DatFlagGround:
			case DatFlagWritable:
			case DatFlagWritableOnce:
			case DatFlagCloth:
			case DatFlagLensHelp:
			case DatFlagUsable:
				if (!file.skip(2)) {
					return false;
				}
				break;

			case DatFlagLight: {
				uint16_t intensity = 0;
				uint16_t color = 0;
				if (!file.getU16(intensity)) {
					return false;
				}
				if (!file.getU16(color)) {
					return false;
				}
				sType->has_light = true;
				sType->light = SpriteLight { static_cast<uint8_t>(intensity), static_cast<uint8_t>(color) };
				break;
			}

			case DatFlagDisplacement: {
				if (format >= DAT_FORMAT_755) {
					uint16_t offset_x = 0;
					uint16_t offset_y = 0;
					if (!file.getU16(offset_x)) {
						return false;
					}
					if (!file.getU16(offset_y)) {
						return false;
					}

					sType->drawoffset_x = offset_x;
					sType->drawoffset_y = offset_y;
				} else {
					sType->drawoffset_x = 8;
					sType->drawoffset_y = 8;
				}
				break;
			}

			case DatFlagElevation: {
				uint16_t draw_height = 0;
				if (!file.getU16(draw_height)) {
					return false;
				}
				sType->draw_height = draw_height;
				break;
			}

			case DatFlagMinimapColor: {
				uint16_t minimap_color = 0;
				if (!file.getU16(minimap_color)) {
					return false;
				}
				sType->minimap_color = minimap_color;
				break;
			}

			case DatFlagMarket: {
				if (!file.skip(6)) {
					return false;
				}
				std::string marketName;
				if (!file.getString(marketName)) {
					return false;
				}
				if (!file.skip(4)) {
					return false;
				}
				break;
			}

			case DatFlagWings: {
				if (!file.skip(16)) {
					return false;
				}
				break;
			}

			default: {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
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
		return true;
	}

	// Helper loop to read all flags for a sprite
	bool LoadMetadataFlags(DatFormat format, FileReadHandle& file, GameSprite* sType, uint32_t sprite_id, wxArrayString& warnings) {
		uint8_t flag = 0xFF; // Initialize to an invalid flag or trailing flag
		uint8_t previous_flag = 0xFF;

		for (int i = 0; i < DatFlagLast; ++i) {
			previous_flag = flag;
			if (!file.getU8(flag)) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
				warnings.push_back(std::format("Metadata: error reading flag for sprite id {}", sprite_id));
#else
				wxString err;
				err.Printf("Metadata: error reading flag for sprite id %u", sprite_id);
				warnings.push_back(err);
#endif
				return false;
			}

			if (flag == DatFlagLast) {
				return true;
			}
			flag = RemapFlag(flag, format);
			if (!ReadFlagData(format, file, sType, flag, previous_flag, warnings)) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
				warnings.push_back(std::format("Metadata: error reading flag data for flag {} for sprite id {}", static_cast<int>(flag), sprite_id));
#else
				wxString err;
				err.Printf("Metadata: error reading flag data for flag %d for sprite id %u", static_cast<int>(flag), sprite_id);
				warnings.push_back(err);
#endif
				return false;
			}
		}
		// Sanity check: If we exit the loop without hitting DatFlagLast, it's potential corruption.
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
		warnings.push_back(std::format("Metadata: corruption warning - flag list exceeded limit (255) without terminator for sprite id {}", sprite_id));
#else
		wxString err;
		err.Printf("Metadata: corruption warning - flag list exceeded limit (255) without terminator for sprite id %d", sprite_id);
		warnings.push_back(err);
#endif
		return true; // We continue even if there was no terminator, as it's just a warning
	}

} // namespace

bool DatLoader::LoadMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	// items.otb has most of the info we need. This only loads the GameSprite metadata
	FileReadHandle file(nstr(datafile.GetFullPath()));

	if (!file.isOk()) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
		// C++20 std::format
		error += std::format("Failed to open {} for reading\nThe error reported was: {}", datafile.GetFullPath().ToStdString(), file.getErrorMessage());
#else
		error += "Failed to open " + datafile.GetFullPath() + " for reading\nThe error reported was:" + wxstr(file.getErrorMessage());
#endif
		return false;
	}

	uint16_t effect_count, distance_count;

	uint32_t datSignature;
	if (!file.getU32(datSignature)) {
		error = "Failed to read dat signature";
		return false;
	}

	// get max id
	if (!file.getU16(manager->item_count) || !file.getU16(manager->creature_count) || !file.getU16(effect_count) || !file.getU16(distance_count)) {
		error = "Failed to read dat header counts";
		return false;
	}

	if (manager->item_count == 0 || manager->creature_count == 0) {
		error = "Invalid dat header counts (zero items or creatures)";
		return false;
	}

	constexpr uint32_t minID = 100; // items start with id 100
	const uint32_t max_id_needed = manager->item_count + manager->creature_count + 1;

	// Pre-size containers to avoid rehashing and frequent allocations
	manager->sprite_space.resize(max_id_needed);
	// image_space is still used for individual sprites, but we'll use a vector
	// We don't know the exact max sprite ID yet, but we can guess or wait for SprLoader
	// For now, let's pre-size it with a reasonable estimate if we can't find total_pics here
	// Actually, DatLoader reads sprite IDs, so it needs image_space to be ready.

	manager->dat_format = manager->client_version->getDatFormatForSignature(datSignature);

	if (manager->dat_format == DAT_FORMAT_UNKNOWN) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
		error = std::format("Unknown dat signature: {:x}", datSignature);
#else
		wxString err;
		err.Printf("Unknown dat signature: %x", datSignature);
		error = err;
#endif
		return false;
	}

	if (!manager->otfi_found) {
		manager->is_extended = manager->dat_format >= DAT_FORMAT_96;
		manager->has_frame_durations = manager->dat_format >= DAT_FORMAT_1050;
		manager->has_frame_groups = manager->dat_format >= DAT_FORMAT_1057;
		manager->has_transparency = manager->client_version->isTransparent();
	}

	uint32_t id = minID;
	const uint32_t maxID = manager->item_count + manager->creature_count;
	while (id <= maxID) {
		auto sTypeUnique = std::make_unique<GameSprite>();
		GameSprite* sType = sTypeUnique.get();
		manager->sprite_space[id] = std::move(sTypeUnique);

		sType->id = id;

		// Load flags
		if (!LoadMetadataFlags(manager->dat_format, file, sType, id, warnings)) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
			error = std::format("Failed to read metadata flags for id {}", id);
#else
			wxString err;
			err.Printf("Failed to read metadata flags for id %u", id);
			error = err;
#endif
			return false;
		}

		// Reads the group count
		uint8_t group_count = 1;
		if (manager->has_frame_groups && id > manager->item_count) {
			if (!file.getU8(group_count)) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
				error = std::format("Failed to read group count for id {}", id);
#else
				wxString err;
				err.Printf("Failed to read group count for id %u", id);
				error = err;
#endif
				return false;
			}
		}

		for (uint32_t k = 0; k < group_count; ++k) {
			if (!ReadSpriteGroup(manager, file, sType, k, warnings)) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
				error = std::format("Failed to read sprite group {} for id {}", k, id);
#else
				wxString err;
				err.Printf("Failed to read sprite group %u for id %u", k, id);
				error = err;
#endif
				return false;
			}
		}
		++id;
	}

	return true;
}

bool DatLoader::ReadSpriteGroup(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, uint32_t group_index, wxArrayString& warnings) {

	// Skipping the group type
	if (manager->has_frame_groups && sType->id > manager->item_count) {
		if (!file.skip(1)) {
			return false;
		}
	}

	uint8_t width = 0, height = 0, layers = 0, pattern_x = 0, pattern_y = 0, pattern_z = 0, frames = 0;

	// Size and GameSprite data
	if (!file.getByte(width)) {
		return false;
	}
	if (!file.getByte(height)) {
		return false;
	}

	// Skipping the exact size
	if ((width > 1) || (height > 1)) {
		if (!file.skip(1)) {
			return false;
		}
	}

	if (!file.getU8(layers)) {
		return false;
	}
	if (!file.getU8(pattern_x)) {
		return false;
	}
	if (!file.getU8(pattern_y)) {
		return false;
	}
	if (manager->dat_format <= DAT_FORMAT_74) {
		pattern_z = 1;
	} else {
		if (!file.getU8(pattern_z)) {
			return false;
		}
	}
	if (!file.getU8(frames)) {
		return false; // Length of animation
	}

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
			if (!file.getByte(async)) {
				return false;
			}
			if (!file.get32(loop_count)) {
				return false;
			}
			if (!file.getSByte(start_frame)) {
				return false;
			}
		}

		if (group_index == 0) {
			sType->animator = std::make_unique<Animator>(frames, start_frame, loop_count, async == 1);
		}

		if (manager->has_frame_durations) {
			for (int i = 0; i < frames; i++) {
				uint32_t min;
				uint32_t max;
				if (!file.getU32(min)) {
					return false;
				}
				if (!file.getU32(max)) {
					return false;
				}
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
			if (!file.getU32(sprite_id)) {
				return false;
			}
		} else {
			uint16_t u16 = 0;
			if (!file.getU16(u16)) {
				return false;
			}
			sprite_id = u16;
		}

		if (group_index == 0) {
			// Validate sprite_id
			if (sprite_id == UINT32_MAX || sprite_id >= MAX_SPRITES || sprite_id + 1 > manager->image_space.max_size()) {
				return false;
			}

			if (sprite_id >= manager->image_space.size()) {
				manager->image_space.resize(sprite_id + 1);
			}

			auto& imgPtr = manager->image_space[sprite_id];
			if (!imgPtr) {
				auto img = newd GameSprite::NormalImage();
				img->id = sprite_id;
				imgPtr = std::unique_ptr<GameSprite::NormalImage>(img);
			}
			sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(imgPtr.get()));
		}
	}

	return true;
}
