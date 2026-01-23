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

#include "main.h"
#include "lua_api_creature.h"
#include "lua_api.h"
#include "../creature.h"
#include "../creatures.h"
#include "../spawn.h"
#include "../tile.h"
#include "../map.h"
#include "../editor.h"
#include "../gui.h"

namespace LuaAPI {

	void registerCreature(sol::state& lua) {
		// Register Direction enum
		lua.new_enum("Direction", "NORTH", NORTH, "EAST", EAST, "SOUTH", SOUTH, "WEST", WEST);

		// Register Creature usertype (expanded from basic in lua_api_tile.cpp)
		lua.new_usertype<Creature>(
			"Creature",
			sol::no_constructor,

			// Properties (read-only)
			"name", sol::property([](Creature* c) -> std::string {
				return c ? c->getName() : "";
			}),
			"isNpc", sol::property([](Creature* c) -> bool {
				return c && c->isNpc();
			}),

			// Properties (read/write)
			"spawnTime", sol::property([](Creature* c) -> int { return c ? c->getSpawnTime() : 0; }, [](Creature* c, int time) {
				if (c) {
					c->setSpawnTime(time);
				} }),
			"direction", sol::property([](Creature* c) -> int { return c ? static_cast<int>(c->getDirection()) : 0; }, [](Creature* c, int dir) {
				if (c && dir >= DIRECTION_FIRST && dir <= DIRECTION_LAST) {
					c->setDirection(static_cast<Direction>(dir));
				} }),

			// Selection
			"isSelected", sol::property([](Creature* c) { return c && c->isSelected(); }),
			"select", [](Creature* c) { if (c){ c->select();
} },
			"deselect", [](Creature* c) { if (c){ c->deselect();
} },

			// String representation
			sol::meta_function::to_string, [](Creature* c) -> std::string {
			if (!c){ return "Creature(invalid)";
}
			std::string dir;
			switch (c->getDirection()) {
				case NORTH: dir = "N"; break;
				case EAST: dir = "E"; break;
				case SOUTH: dir = "S"; break;
				case WEST: dir = "W"; break;
				default: dir = "?"; break;
			}
			return "Creature(\"" + c->getName() + "\", dir=" + dir +
				   ", spawn=" + std::to_string(c->getSpawnTime()) + "s)"; }
		);

		// Register Spawn usertype (expanded from basic in lua_api_tile.cpp)
		lua.new_usertype<Spawn>(
			"Spawn",
			sol::no_constructor,

			// Properties (read/write)
			"size", sol::property([](Spawn* s) -> int { return s ? s->getSize() : 0; }, [](Spawn* s, int size) {
				if (s && size > 0 && size < 100) {
					s->setSize(size);
				} }),
			// Alias for size
			"radius", sol::property([](Spawn* s) -> int { return s ? s->getSize() : 0; }, [](Spawn* s, int size) {
				if (s && size > 0 && size < 100) {
					s->setSize(size);
				} }),

			// Selection
			"isSelected", sol::property([](Spawn* s) { return s && s->isSelected(); }),
			"select", [](Spawn* s) { if (s){ s->select();
} },
			"deselect", [](Spawn* s) { if (s){ s->deselect();
} },

			// String representation
			sol::meta_function::to_string, [](Spawn* s) -> std::string {
			if (!s){ return "Spawn(invalid)";
}
			return "Spawn(radius=" + std::to_string(s->getSize()) + ")"; }
		);

		// Helper function to check if a creature type exists
		lua["creatureExists"] = [](const std::string& name) -> bool {
			return g_creatures[name] != nullptr;
		};

		// Helper function to check if a creature is an NPC by name
		lua["isNpcType"] = [](const std::string& name) -> bool {
			CreatureType* type = g_creatures[name];
			return type && type->isNpc;
		};
	}

} // namespace LuaAPI
