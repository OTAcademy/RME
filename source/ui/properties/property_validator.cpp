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
