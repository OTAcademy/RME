//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/property_validator.h"
#include "ui/dialog_util.h"
#include "game/house.h"
#include "map/map.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/complexitem.h"
#include "game/creature.h"

static constexpr int OUTFIT_COLOR_MAX = 133;

bool PropertyValidator::validateItemProperties(wxWindow* parent, int uid, int aid, int tier) {
	if ((uid < 1000 || uid > 0xFFFF) && uid != 0) {
		DialogUtil::PopupDialog(parent, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
		return false;
	}
	if ((aid < 100 || aid > 0xFFFF) && aid != 0) {
		DialogUtil::PopupDialog(parent, "Error", "Action ID must be between 100 and 65535.", wxOK);
		return false;
	}
	if (tier < 0 || tier > 0xFF) {
		DialogUtil::PopupDialog(parent, "Error", "Item tier must be between 0 and 255.", wxOK);
		return false;
	}
	return true;
}

bool PropertyValidator::validateDoorProperties(wxWindow* parent, const Map* map, const Tile* tile, const Door* door, uint8_t door_id) {
	if (g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
		if (tile && tile->isHouseTile()) {
			const House* house = map->houses.getHouse(tile->getHouseID());
			if (house) {
				Position pos = house->getDoorPositionByID(door_id);
				if (pos != Position() && pos != tile->getPosition()) {
					int ret = DialogUtil::PopupDialog(parent, "Warning", "This doorid conflicts with another one in this house, are you sure you want to continue?", wxYES | wxNO);
					if (ret == wxID_NO) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool PropertyValidator::validateTeleportProperties(wxWindow* parent, const Map* map, const Position& dest) {
	if (map->getTile(dest) == nullptr || map->getTile(dest)->isBlocking()) {
		int ret = DialogUtil::PopupDialog(parent, "Warning", "This teleport leads nowhere, or to an invalid location. Do you want to change the destination?", wxYES | wxNO);
		if (ret == wxID_YES) {
			return false;
		}
	}
	return true;
}

bool PropertyValidator::validatePodiumProperties(wxWindow* parent, int tier, const Outfit& outfit, int lookMount) {
	if (tier < 0 || tier > 0xFF) {
		DialogUtil::PopupDialog(parent, "Error", "Item tier must be between 0 and 255.", wxOK);
		return false;
	}

	if (outfit.lookType < 0 || outfit.lookType > 0xFFFF || lookMount < 0 || lookMount > 0xFFFF) {
		DialogUtil::PopupDialog(parent, "Error", "LookType and Mount must be between 0 and 65535.", wxOK);
		return false;
	}

	if (outfit.lookHead < 0 || outfit.lookHead > OUTFIT_COLOR_MAX || outfit.lookBody < 0 || outfit.lookBody > OUTFIT_COLOR_MAX || outfit.lookLegs < 0 || outfit.lookLegs > OUTFIT_COLOR_MAX || outfit.lookFeet < 0 || outfit.lookFeet > OUTFIT_COLOR_MAX || outfit.lookMountHead < 0 || outfit.lookMountHead > OUTFIT_COLOR_MAX || outfit.lookMountBody < 0 || outfit.lookMountBody > OUTFIT_COLOR_MAX || outfit.lookMountLegs < 0 || outfit.lookMountLegs > OUTFIT_COLOR_MAX || outfit.lookMountFeet < 0 || outfit.lookMountFeet > OUTFIT_COLOR_MAX) {
		wxString response = "Outfit and mount colors must be between 0 and ";
		response << i2ws(OUTFIT_COLOR_MAX) << ".";
		DialogUtil::PopupDialog(parent, "Error", response, wxOK);
		return false;
	}

	if (outfit.lookAddon < 0 || outfit.lookAddon > 3) {
		DialogUtil::PopupDialog(parent, "Error", "Addons value must be between 0 and 3.", wxOK);
		return false;
	}

	return true;
}

bool PropertyValidator::validateTextProperties(wxWindow* parent, const Item* item, const std::string& text) {
	if (text.length() >= 0xFFFF) {
		DialogUtil::PopupDialog(parent, "Error", "Text is longer than 65535 characters, this is not supported by OpenTibia. Reduce the length of the text.", wxOK);
		return false;
	}
	if (item->canHoldText() && text.length() > item->getMaxWriteLength()) {
		int ret = DialogUtil::PopupDialog(parent, "Error", "Text is longer than the maximum supported length of this book type, do you still want to change it?", wxYES | wxNO);
		if (ret != wxID_YES) {
			return false;
		}
	}
	return true;
}
