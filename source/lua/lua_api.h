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

#ifndef RME_LUA_API_H
#define RME_LUA_API_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// Forward declarations
class Tile;

// Forward declarations for API modules
namespace LuaAPI {
	// Register all APIs with the Lua state
	void registerAll(sol::state& lua);

	// Individual API registration functions
	void registerApp(sol::state& lua);
	void registerDialog(sol::state& lua);

	void registerPosition(sol::state& lua);
	void registerItem(sol::state& lua);
	void registerTile(sol::state& lua);
	void registerMap(sol::state& lua);
	void registerSelection(sol::state& lua);

	// Called by tile modification functions to track changes
	void markTileForUndo(Tile* tile);

	void registerColor(sol::state& lua);
	void registerCreature(sol::state& lua);
	void registerBrush(sol::state& lua);
	void registerImage(sol::state& lua);
	void registerJson(sol::state& lua);
	void registerHttp(sol::state& lua);

	// Procedural generation APIs
	void registerNoise(sol::state& lua);
	void registerAlgo(sol::state& lua);
	void registerGeo(sol::state& lua);
}

#endif // RME_LUA_API_H
