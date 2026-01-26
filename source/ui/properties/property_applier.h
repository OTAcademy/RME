//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_PROPERTY_APPLIER_H
#define RME_PROPERTY_APPLIER_H

#include "app/main.h"
#include "map/position.h"

class Item;
class Door;
class Teleport;

class PropertyApplier {
public:
	static void applyItemProperties(Item* item, int count, int uid, int aid, int tier);
	static void applyDoorProperties(Door* door, uint8_t door_id);
	static void applyTeleportProperties(Teleport* teleport, const Position& dest);
};

#endif
