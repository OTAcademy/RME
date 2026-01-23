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
#include "lua_api.h"

namespace LuaAPI {

	void registerAll(sol::state& lua) {
		// Register base types first (order matters for dependencies)

		// Position is used by Tile and Map
		registerPosition(lua);

		// Item is used by Tile
		registerItem(lua);

		// Color is a standalone type
		registerColor(lua);

		// Creature/Spawn are used by Tile (must come before Tile)
		registerCreature(lua);

		// Tile is used by Map and Selection
		registerTile(lua);

		// Map uses Tile and Position
		registerMap(lua);

		// Selection uses Tile
		registerSelection(lua);

		// Must come after Map and Selection so app.map/app.selection work
		registerApp(lua);

		// Register Dialog class
		registerDialog(lua);

		registerBrush(lua);

		// Register Image class
		registerImage(lua);

		// Register JSON helper
		registerJson(lua);

		// Register HTTP client
		registerHttp(lua);

		// Register procedural generation APIs
		registerNoise(lua);
		registerAlgo(lua);
		registerGeo(lua);
	}

}
