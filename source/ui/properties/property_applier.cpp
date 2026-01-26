//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/property_applier.h"
#include "game/item.h"
#include "game/complexitem.h"

void PropertyApplier::applyItemProperties(Item* item, int count, int uid, int aid, int tier) {
	if (item->isStackable() || item->isCharged()) {
		item->setSubtype(count);
	}
	item->setUniqueID(uid);
	item->setActionID(aid);
	item->setTier(tier);
}

void PropertyApplier::applyDoorProperties(Door* door, uint8_t door_id) {
	door->setDoorID(door_id);
}

void PropertyApplier::applyTeleportProperties(Teleport* teleport, const Position& dest) {
	teleport->setDestination(dest);
}
