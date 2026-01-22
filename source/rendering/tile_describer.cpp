#include "rendering/tile_describer.h"
#include "main.h"
#include "definitions.h"
#include "items.h"
#include "tile.h"
#include "spawn.h"
#include "creature.h"
#include "item.h"
#include "common.h" // for wxstr

wxString TileDescriber::GetDescription(Tile* tile, bool showSpawns, bool showCreatures) {
	wxString ss;
	if (tile) {
		if (tile->spawn && showSpawns) {
			ss << "Spawn radius: " << tile->spawn->getSize();
		} else if (tile->creature && showCreatures) {
			ss << (tile->creature->isNpc() ? "NPC" : "Monster");
			ss << " \"" << wxstr(tile->creature->getName()) << "\" spawntime: " << tile->creature->getSpawnTime();
		} else if (Item* item = tile->getTopItem()) {
			ss << "Item \"" << wxstr(item->getName()) << "\"";
			ss << " id:" << item->getID();
			ss << " cid:" << item->getClientID();
			if (item->getUniqueID()) {
				ss << " uid:" << item->getUniqueID();
			}
			if (item->getActionID()) {
				ss << " aid:" << item->getActionID();
			}
			if (item->hasWeight()) {
				wxString s;
				s.Printf("%.2f", item->getWeight());
				ss << " weight: " << s;
			}
		} else {
			ss << "Nothing";
		}
	} else {
		ss << "Nothing";
	}
	return ss;
}
