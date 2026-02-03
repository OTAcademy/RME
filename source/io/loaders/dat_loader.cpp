#include "io/loaders/dat_loader.h"

#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "util/common.h"
#include <memory>

bool DatLoader::LoadMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	// items.otb has most of the info we need. This only loads the GameSprite metadata
	FileReadHandle file(nstr(datafile.GetFullPath()));

	if (!file.isOk()) {
		error += "Failed to open " + datafile.GetFullPath() + " for reading\nThe error reported was:" + wxstr(file.getErrorMessage());
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
	// We don't load distance/effects, if we would, just add effect_count & distance_count here
	const uint32_t maxID = manager->item_count + manager->creature_count;

	manager->dat_format = manager->client_version->getDatFormatForSignature(datSignature);

	if (!manager->otfi_found) {
		manager->is_extended = manager->dat_format >= DAT_FORMAT_96;
		manager->has_frame_durations = manager->dat_format >= DAT_FORMAT_1050;
		manager->has_frame_groups = manager->dat_format >= DAT_FORMAT_1057;
	}

	uint16_t id = minID;
	// loop through all ItemDatabase until we reach the end of file
	while (id <= maxID) {
		auto sTypeUnique = std::make_unique<GameSprite>();
		GameSprite* sType = sTypeUnique.get();
		manager->sprite_space[id] = std::move(sTypeUnique);

		sType->id = id;

		// Load the sprite flags
		if (!LoadMetadataFlags(manager, file, sType, error, warnings)) {
			wxString msg;
			msg << "Failed to load flags for sprite " << sType->id;
			warnings.push_back(msg);
		}

		// Reads the group count
		uint8_t group_count = 1;
		if (manager->has_frame_groups && id > manager->item_count) {
			file.getU8(group_count);
		}

		for (uint32_t k = 0; k < group_count; ++k) {
			// Skipping the group type
			if (manager->has_frame_groups && id > manager->item_count) {
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

			file.getU8(layers); // Number of blendframes (some sprites consist of several merged sprites)
			file.getU8(pattern_x);
			file.getU8(pattern_y);
			if (manager->dat_format <= DAT_FORMAT_74) {
				pattern_z = 1;
			} else {
				file.getU8(pattern_z);
			}
			file.getU8(frames); // Length of animation

			if (k == 0) {
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

				if (k == 0) {
					sType->animator = std::make_unique<Animator>(frames, start_frame, loop_count, async == 1);
				}

				if (manager->has_frame_durations) {
					for (int i = 0; i < frames; i++) {
						uint32_t min;
						uint32_t max;
						file.getU32(min);
						file.getU32(max);
						if (k == 0) {
							FrameDuration* frame_duration = sType->animator->getFrameDuration(i);
							frame_duration->setValues(int(min), int(max));
						}
					}
					if (k == 0) {
						sType->animator->reset();
					}
				}
			}

			uint32_t numsprites = (int)width * (int)height * (int)layers * (int)pattern_x * (int)pattern_y * pattern_z * (int)frames;
			if (k == 0) {
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

				if (k == 0) {
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
		++id;
	}

	return true;
}

uint8_t DatLoader::RemapFlag(uint8_t flag, DatFormat format) {
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

bool DatLoader::ReadFlagData(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, uint8_t flag, wxArrayString& warnings) {
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
			if (manager->dat_format >= DAT_FORMAT_755) {
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
			wxString err;
			// err << "Metadata: Unknown flag: " << i2ws(flag) << ". Previous flag: " << i2ws(prev_flag) << ".";
			err << "Metadata: Unknown flag: " << i2ws(flag);
			warnings.push_back(err);
			break;
		}
	}
	return true;
}

bool DatLoader::LoadMetadataFlags(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings) {
	uint8_t prev_flag = 0;
	uint8_t flag = DatFlagLast;

	for (int i = 0; i < DatFlagLast; ++i) {
		prev_flag = flag;
		file.getU8(flag);

		if (flag == DatFlagLast) {
			return true;
		}

		flag = RemapFlag(flag, manager->dat_format);
		ReadFlagData(manager, file, sType, flag, warnings);
	}

	return true;
}
