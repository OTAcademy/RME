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
#include "lua_api_position.h"
#include "../position.h"

namespace LuaAPI {

	void registerPosition(sol::state& lua) {
		// Register Position usertype
		lua.new_usertype<Position>(
			"Position",
			// Constructors
			sol::constructors<
				Position(),
				Position(int, int, int)>(),

			// Also allow construction from table: Position{x=100, y=200, z=7}
			sol::call_constructor, sol::factories([](int x, int y, int z) { return Position(x, y, z); }, [](sol::table t) {
				int x = t.get_or("x", 0);
				int y = t.get_or("y", 0);
				int z = t.get_or("z", 0);
				return Position(x, y, z); }),

			// Properties
			"x", &Position::x,
			"y", &Position::y,
			"z", &Position::z,

			// Methods
			"isValid", &Position::isValid,

			// Operators
			sol::meta_function::equal_to, [](const Position& a, const Position& b) { return a == b; },
			sol::meta_function::addition, [](const Position& a, const Position& b) { return a + b; },
			sol::meta_function::subtraction, [](const Position& a, const Position& b) { return a - b; },
			sol::meta_function::to_string, [](const Position& pos) { return "Position(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")"; }
		);
	}

} // namespace LuaAPI
