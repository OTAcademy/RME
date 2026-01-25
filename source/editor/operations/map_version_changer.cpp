#include "editor/operations/map_version_changer.h"

#include "editor/editor.h"
#include "editor/action_queue.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "app/managers/version_manager.h"
#include "app/application.h"
#include "game/creature.h"
#include "game/items.h"
#include "map/tile.h"

struct MapConversionContext {
	struct CreatureInfo {
		std::string name;
		bool is_npc;
		Outfit outfit;
	};
	using CreatureMap = std::map<std::string, CreatureInfo>;
	CreatureMap creature_types;

	void operator()(Map& map, Tile* tile, long long done) {
		if (tile->creature) {
			CreatureMap::iterator f = creature_types.find(tile->creature->getName());
			if (f == creature_types.end()) {
				CreatureInfo info = {
					tile->creature->getName(),
					tile->creature->isNpc(),
					tile->creature->getLookType()
				};
				creature_types[tile->creature->getName()] = info;
			}
		}
	}
};

bool MapVersionChanger::changeMapVersion(wxWindow* parent, Editor& editor, MapVersion new_ver) {
	Map& map = editor.map;
	MapVersion old_ver = map.getVersion();

	if (new_ver.client != old_ver.client) {
		if (g_gui.GetOpenMapCount() > 1) {
			DialogUtil::PopupDialog(parent, "Error", "You can not change editor version with multiple maps open", wxOK);
			return false;
		}
		wxString error;
		wxArrayString warnings;

		// Switch version
		// Switch version
		editor.selection.clear();
		editor.actionQueue->clear();

		if (new_ver.client < old_ver.client) {
			int ret = DialogUtil::PopupDialog(parent, "Notice", "Converting to a previous version may have serious side-effects, are you sure you want to do this?", wxYES | wxNO);
			if (ret != wxID_YES) {
				return false;
			}
			UnnamedRenderingLock();

			// Remember all creatures types on the map
			MapConversionContext conversion_context;
			foreach_TileOnMap(map, conversion_context);

			// Perform the conversion
			map.convert(new_ver, true);

			// Load the new version
			if (!g_version.LoadVersion(new_ver.client, error, warnings)) {
				DialogUtil::ListDialog(parent, "Warnings", warnings);
				DialogUtil::PopupDialog(parent, "Map Loader Error", error, wxOK);
				DialogUtil::PopupDialog(parent, "Conversion Error", "Could not convert map. The map will now be closed.", wxOK);

				return false;
			}

			// Remove all creatures that were present are present in the new version
			std::erase_if(conversion_context.creature_types, [](const auto& pair) {
				return g_creatures[pair.first] != nullptr;
			});

			if (!conversion_context.creature_types.empty()) {
				int add = DialogUtil::PopupDialog(parent, "Unrecognized creatures", "There were creatures on the old version that are not present in this and were on the map, do you want to add them to this version as well?", wxYES | wxNO);
				if (add == wxID_YES) {
					for (const auto& [name, info] : conversion_context.creature_types) {
						g_creatures.addCreatureType(info.name, info.is_npc, info.outfit);
					}
				}
			}

			map.cleanInvalidTiles(true);
		} else {
			UnnamedRenderingLock();
			if (!g_version.LoadVersion(new_ver.client, error, warnings)) {
				DialogUtil::ListDialog(parent, "Warnings", warnings);
				DialogUtil::PopupDialog(parent, "Map Loader Error", error, wxOK);
				DialogUtil::PopupDialog(parent, "Conversion Error", "Could not convert map. The map will now be closed.", wxOK);

				return false;
			}
			map.convert(new_ver, true);
		}
	} else {
		map.convert(new_ver, true);
	}
	return true;
}
