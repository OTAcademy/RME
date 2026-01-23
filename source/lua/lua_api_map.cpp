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
#include "lua_api_map.h"
#include "../map.h"
#include "../basemap.h"
#include "../tile.h"
#include "../position.h"
#include "../gui.h"
#include "../editor.h"

namespace LuaAPI {

	// Custom iterator for Map tiles to use in Lua for-loops
	class LuaMapTileIterator {
	public:
		LuaMapTileIterator(Map* map) :
			map(map), started(false) {
			if (map) {
				iter = map->begin();
				endIter = map->end();
			}
		}

		std::tuple<sol::object, sol::object> next(sol::this_state ts) {
			sol::state_view lua(ts);

			if (!map) {
				return std::make_tuple(sol::nil, sol::nil);
			}

			// Find next valid tile
			while (iter != endIter) {
				TileLocation* loc = *iter;
				++iter;

				if (loc && loc->get()) {
					Tile* tile = loc->get();
					return std::make_tuple(
						sol::make_object(lua, tile),
						sol::make_object(lua, tile)
					);
				}
			}

			return std::make_tuple(sol::nil, sol::nil);
		}

	private:
		Map* map;
		MapIterator iter;
		MapIterator endIter;
		bool started;
	};

	// Iterator for Spawns
	class LuaMapSpawnIterator {
	public:
		LuaMapSpawnIterator(Map* map) :
			map(map) {
			if (map) {
				iter = map->spawns.begin();
				endIter = map->spawns.end();
			}
		}

		Tile* next() {
			if (!map) {
				return nullptr;
			}

			if (iter != endIter) {
				Position pos = *iter;
				++iter;
				return map->getTile(pos);
			}
			return nullptr;
		}

	private:
		Map* map;
		SpawnPositionList::const_iterator iter;
		SpawnPositionList::const_iterator endIter;
	};

	void registerMap(sol::state& lua) {
		// Register the iterator type
		lua.new_usertype<LuaMapTileIterator>("MapTileIterator", sol::no_constructor, "next", &LuaMapTileIterator::next);

		// Register Spawn iterator
		lua.new_usertype<LuaMapSpawnIterator>("MapSpawnIterator", sol::no_constructor, "next", &LuaMapSpawnIterator::next);

		// Register Map usertype
		lua.new_usertype<Map>(
			"Map",
			// No public constructor - maps are obtained from app.map
			sol::no_constructor,

			// Properties (read-only)
			"name", sol::property(&Map::getName),
			"filename", sol::property(&Map::getFilename),
			"description", sol::property(&Map::getMapDescription),
			"width", sol::property(&Map::getWidth),
			"height", sol::property(&Map::getHeight),
			"houseFilename", sol::property(&Map::getHouseFilename),
			"spawnFilename", sol::property(&Map::getSpawnFilename),
			"hasFile", sol::property(&Map::hasFile),
			"hasChanged", sol::property(&Map::hasChanged),
			"tileCount", sol::property([](Map* map) -> uint64_t {
				return map ? map->getTileCount() : 0;
			}),

			// Get tile methods
			"getTile", sol::overload([](Map* map, int x, int y, int z) -> Tile* { return map ? map->getTile(x, y, z) : nullptr; }, [](Map* map, const Position& pos) -> Tile* { return map ? map->getTile(pos) : nullptr; }),

			// Get or create tile (for adding content to empty positions)
			"getOrCreateTile", [](Map* map, sol::variadic_args va) -> Tile* {
			if (!map){ return nullptr;
}

			Position pos;
			if (va.size() == 1 && va[0].is<Position>()) {
				pos = va[0].as<Position>();
			} else if (va.size() == 3) {
				pos.x = va[0].as<int>();
				pos.y = va[1].as<int>();
				pos.z = va[2].as<int>();
			} else {
				throw sol::error("getOrCreateTile expects (x, y, z) or (Position)");
			}

			return map->getOrCreateTile(pos); },

			// Tiles iterator - allows: for tile in map.tiles do ... end
			"tiles", sol::property([](Map* map, sol::this_state ts) {
				sol::state_view lua(ts);

				// Return an iterator function
				auto iterator = std::make_shared<LuaMapTileIterator>(map);

				// Return the iterator function that Lua will call repeatedly
				return sol::make_object(lua, [iterator](sol::this_state ts) {
					return iterator->next(ts);
				});
			}),

			// Spawns iterator - allows: for tile in map.spawns do ... end
			"spawns", sol::property([](Map* map, sol::this_state ts) {
				sol::state_view lua(ts);

				auto iterator = std::make_shared<LuaMapSpawnIterator>(map);

				return sol::make_object(lua, [iterator]() -> Tile* {
					return iterator->next();
				});
			}),

			// String representation
			sol::meta_function::to_string, [](Map* map) {
			if (!map){ return std::string("Map(invalid)");
}
			return "Map(\"" + map->getName() + "\", " +
				   std::to_string(map->getWidth()) + "x" +
				   std::to_string(map->getHeight()) + ")"; }
		);
	}

} // namespace LuaAPI
