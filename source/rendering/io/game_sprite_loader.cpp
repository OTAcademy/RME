//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "rendering/io/game_sprite_loader.h"
#include "rendering/core/graphics.h"
#include "io/filehandle.h"
#include "app/settings.h"
#include "map/otml.h"
#include <wx/dir.h>
#include <memory>

bool GameSpriteLoader::LoadOTFI(GraphicManager* manager, const wxFileName& filename, wxString& error, wxArrayString& warnings) {
	wxDir dir(filename.GetFullPath());
	wxString otfi_file;

	manager->otfi_found = false;

	if (dir.GetFirst(&otfi_file, "*.otfi", wxDIR_FILES)) {
		wxFileName otfi(filename.GetFullPath(), otfi_file);
		OTMLDocumentPtr doc = OTMLDocument::parse(otfi.GetFullPath().ToStdString());
		if (doc->size() == 0 || !doc->hasChildAt("DatSpr")) {
			error += "'DatSpr' tag not found";
			return false;
		}

		OTMLNodePtr node = doc->get("DatSpr");
		manager->is_extended = node->valueAt<bool>("extended");
		manager->has_transparency = node->valueAt<bool>("transparency");
		manager->has_frame_durations = node->valueAt<bool>("frame-durations");
		manager->has_frame_groups = node->valueAt<bool>("frame-groups");
		std::string metadata = node->valueAt<std::string>("metadata-file", std::string(ASSETS_NAME) + ".dat");
		std::string sprites = node->valueAt<std::string>("sprites-file", std::string(ASSETS_NAME) + ".spr");
		manager->metadata_file = wxFileName(filename.GetFullPath(), wxString(metadata));
		manager->sprites_file = wxFileName(filename.GetFullPath(), wxString(sprites));
		manager->otfi_found = true;
	}

	if (!manager->otfi_found) {
		manager->is_extended = false;
		manager->has_transparency = false;
		manager->has_frame_durations = false;
		manager->has_frame_groups = false;
		manager->metadata_file = wxFileName(filename.GetFullPath(), wxString(ASSETS_NAME) + ".dat");
		manager->sprites_file = wxFileName(filename.GetFullPath(), wxString(ASSETS_NAME) + ".spr");
	}

	return true;
}

bool GameSpriteLoader::LoadSpriteMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
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

	uint32_t minID = 100; // items start with id 100
	// We don't load distance/effects, if we would, just add effect_count & distance_count here
	uint32_t maxID = manager->item_count + manager->creature_count;

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
		if (!LoadSpriteMetadataFlags(manager, file, sType, error, warnings)) {
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

			// Size and GameSprite data
			file.getByte(sType->width);
			file.getByte(sType->height);

			// Skipping the exact size
			if ((sType->width > 1) || (sType->height > 1)) {
				file.skip(1);
			}

			file.getU8(sType->layers); // Number of blendframes (some sprites consist of several merged sprites)
			file.getU8(sType->pattern_x);
			file.getU8(sType->pattern_y);
			if (manager->dat_format <= DAT_FORMAT_74) {
				sType->pattern_z = 1;
			} else {
				file.getU8(sType->pattern_z);
			}
			file.getU8(sType->frames); // Length of animation

			if (sType->frames > 1) {
				uint8_t async = 0;
				int loop_count = 0;
				int8_t start_frame = 0;
				if (manager->has_frame_durations) {
					file.getByte(async);
					file.get32(loop_count);
					file.getSByte(start_frame);
				}
				sType->animator = std::make_unique<Animator>(sType->frames, start_frame, loop_count, async == 1);
				if (manager->has_frame_durations) {
					for (int i = 0; i < sType->frames; i++) {
						uint32_t min;
						uint32_t max;
						file.getU32(min);
						file.getU32(max);
						FrameDuration* frame_duration = sType->animator->getFrameDuration(i);
						frame_duration->setValues(static_cast<int>(min), static_cast<int>(max));
					}
					sType->animator->reset();
				}
			}

			sType->numsprites = static_cast<int>(sType->width) * static_cast<int>(sType->height) * static_cast<int>(sType->layers) * static_cast<int>(sType->pattern_x) * static_cast<int>(sType->pattern_y) * sType->pattern_z * static_cast<int>(sType->frames);

			// Read the sprite ids
			for (uint32_t i = 0; i < sType->numsprites; ++i) {
				uint32_t sprite_id;
				if (manager->is_extended) {
					file.getU32(sprite_id);
				} else {
					uint16_t u16 = 0;
					file.getU16(u16);
					sprite_id = u16;
				}

				auto [it, inserted] = manager->image_space.try_emplace(sprite_id, nullptr);
				if (inserted) {
					auto img = std::make_unique<GameSprite::NormalImage>();
					img->id = sprite_id;
					it->second = std::move(img);
				}
				sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(it->second.get()));
			}
		}
		++id;
	}

	return true;
}

bool GameSpriteLoader::LoadSpriteMetadataFlags(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings) {
	uint8_t prev_flag = 0;
	uint8_t flag = DatFlagLast;

	for (int i = 0; i < DatFlagLast; ++i) {
		prev_flag = flag;
		file.getU8(flag);

		if (flag == DatFlagLast) {
			return true;
		}
		if (manager->dat_format >= DAT_FORMAT_1010) {
			/* In 10.10+ all attributes from 16 and up were
			 * incremented by 1 to make space for 16 as
			 * "No Movement Animation" flag.
			 */
			if (flag == 16) {
				flag = DatFlagNoMoveAnimation;
			} else if (flag > 16) {
				flag -= 1;
			}
		} else if (manager->dat_format >= DAT_FORMAT_86) {
			/* Default attribute values follow
			 * the format of 8.6-9.86.
			 * Therefore no changes here.
			 */
		} else if (manager->dat_format >= DAT_FORMAT_78) {
			/* In 7.80-8.54 all attributes from 8 and higher were
			 * incremented by 1 to make space for 8 as
			 * "Item Charges" flag.
			 */
			if (flag == 8) {
				flag = DatFlagChargeable;
			} else if (flag > 8) {
				flag -= 1;
			}
		} else if (manager->dat_format >= DAT_FORMAT_755) {
			/* In 7.55-7.72 attributes 23 is "Floor Change". */
			if (flag == 23) {
				flag = DatFlagFloorChange;
			}
		} else if (manager->dat_format >= DAT_FORMAT_74) {
			/* In 7.4-7.5 attribute "Ground Border" did not exist
			 * attributes 1-15 have to be adjusted.
			 * Several other changes in the format.
			 */
			if (flag > 0 && flag <= 15) {
				flag += 1;
			} else if (flag == 16) {
				flag = DatFlagLight;
			} else if (flag == 17) {
				flag = DatFlagFloorChange;
			} else if (flag == 18) {
				flag = DatFlagFullGround;
			} else if (flag == 19) {
				flag = DatFlagElevation;
			} else if (flag == 20) {
				flag = DatFlagDisplacement;
			} else if (flag == 22) {
				flag = DatFlagMinimapColor;
			} else if (flag == 23) {
				flag = DatFlagRotateable;
			} else if (flag == 24) {
				flag = DatFlagLyingCorpse;
			} else if (flag == 25) {
				flag = DatFlagHangable;
			} else if (flag == 26) {
				flag = DatFlagHookSouth;
			} else if (flag == 27) {
				flag = DatFlagHookEast;
			} else if (flag == 28) {
				flag = DatFlagAnimateAlways;
			}

			/* "Multi Use" and "Force Use" are swapped */
			if (flag == DatFlagMultiUse) {
				flag = DatFlagForceUse;
			} else if (flag == DatFlagForceUse) {
				flag = DatFlagMultiUse;
			}
		}

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
				SpriteLight light;
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
				err << "Metadata: Unknown flag: " << i2ws(flag) << ". Previous flag: " << i2ws(prev_flag) << ".";
				warnings.push_back(err);
				break;
			}
		}
	}

	return true;
}

bool GameSpriteLoader::LoadSpriteData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
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
	for (uint32_t i = 0; i < total_pics; ++i) {
		uint32_t index;
		safe_get(U32, index);
		sprite_indexes.push_back(index);
	}

	// Now read individual sprites
	int id = 1;
	for (std::vector<uint32_t>::iterator sprite_iter = sprite_indexes.begin(); sprite_iter != sprite_indexes.end(); ++sprite_iter, ++id) {
		uint32_t index = *sprite_iter + 3;
		fh.seek(index);
		uint16_t size;
		safe_get(U16, size);

		auto it = manager->image_space.find(id);
		if (it != manager->image_space.end()) {
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
	}
#undef safe_get
	manager->unloaded = false;
	return true;
}

bool GameSpriteLoader::LoadSpriteDump(GraphicManager* manager, std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id) {
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
