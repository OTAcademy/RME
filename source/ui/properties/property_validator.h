//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_PROPERTY_VALIDATOR_H
#define RME_PROPERTY_VALIDATOR_H

#include "app/main.h"
#include "map/position.h"

class Map;
class Tile;
class Item;
class Door;
class Teleport;

class PropertyValidator {
public:
	static bool validateItemProperties(wxWindow* parent, int uid, int aid, int tier);
	static bool validateDoorProperties(wxWindow* parent, const Map* map, const Tile* tile, const Door* door, uint8_t door_id);
	static bool validateTeleportProperties(wxWindow* parent, const Map* map, const Position& dest);
};

#endif
