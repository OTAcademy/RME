//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "brushes/creature/creature_brush.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "map/tile.h"
#include "game/creature.h"
#include "map/basemap.h"
#include "game/spawn.h"
#include "game/items.h"

//=============================================================================
// Creature brush

CreatureBrush::CreatureBrush(CreatureType* type) :
	Brush(),
	creature_type(type) {
	ASSERT(type->brush == nullptr);
	type->brush = this;
}

CreatureBrush::~CreatureBrush() {
	////
}

int CreatureBrush::getLookID() const {
	return 0;
}

Sprite* CreatureBrush::getSprite() const {
	if (creature_type) {
		const Outfit& outfit = creature_type->outfit;
		if (outfit.lookItem != 0) {
			ItemType& it = g_items[outfit.lookItem];
			return it.sprite;
		}

		if (outfit.lookType != 0) {
			if (!creature_sprite_wrapper) {
				GameSprite* gs = g_gui.gfx.getCreatureSprite(outfit.lookType);
				if (gs) {
					creature_sprite_wrapper = std::make_unique<CreatureSprite>(gs, outfit);
				}
			}
			return creature_sprite_wrapper.get();
		}
	}
	return nullptr;
}

std::string CreatureBrush::getName() const {
	if (creature_type) {
		return creature_type->name;
	}
	return "Creature Brush";
}

bool CreatureBrush::canDraw(BaseMap* map, const Position& position) const {
	Tile* tile = map->getTile(position);
	if (creature_type && tile && !tile->isBlocking()) {
		if (tile->getLocation()->getSpawnCount() != 0 || g_settings.getInteger(Config::AUTO_CREATE_SPAWN)) {
			if (tile->isPZ()) {
				if (creature_type->isNpc) {
					return true;
				}
			} else {
				return true;
			}
		}
	}
	return false;
}

void CreatureBrush::undraw(BaseMap* map, Tile* tile) {
	tile->creature.reset();
}

void CreatureBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	ASSERT(parameter);
	draw_creature(map, tile);
}

void CreatureBrush::draw_creature(BaseMap* map, Tile* tile) {
	if (canDraw(map, tile->getPosition())) {
		undraw(map, tile);
		if (creature_type) {
			if (tile->spawn == nullptr && tile->getLocation()->getSpawnCount() == 0) {
				// manually place spawn on location
				tile->spawn = std::make_unique<Spawn>(1);
			}
			tile->creature = std::make_unique<Creature>(creature_type);
			tile->creature->setSpawnTime(g_gui.GetSpawnTime());
		}
	}
}
