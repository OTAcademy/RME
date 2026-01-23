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
#include "lua_api_color.h"

namespace LuaAPI {

void registerColor(sol::state& lua) {
	// Register LuaColor as "Color" usertype
	lua.new_usertype<LuaColor>("Color",
		// Constructors
		sol::constructors<
			LuaColor(),                                    // Color()
			LuaColor(uint8_t, uint8_t, uint8_t),           // Color(r, g, b)
			LuaColor(uint8_t, uint8_t, uint8_t, uint8_t)   // Color(r, g, b, a)
		>(),

		// Alternative constructor from table: Color{red=255, green=0, blue=0}
		sol::call_constructor, sol::factories(
			// Default constructor
			[]() { return LuaColor(); },
			// RGB constructor
			[](uint8_t r, uint8_t g, uint8_t b) { return LuaColor(r, g, b); },
			// RGBA constructor
			[](uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return LuaColor(r, g, b, a); },
			// Table constructor
			[](sol::table t) {
				LuaColor c;
				c.red = t.get_or<uint8_t>("red", 0);
				c.green = t.get_or<uint8_t>("green", 0);
				c.blue = t.get_or<uint8_t>("blue", 0);
				c.alpha = t.get_or<uint8_t>("alpha", 255);
				return c;
			}
		),

		// Properties (read/write)
		"red", &LuaColor::red,
		"green", &LuaColor::green,
		"blue", &LuaColor::blue,
		"alpha", &LuaColor::alpha,

		// Shorthand aliases
		"r", &LuaColor::red,
		"g", &LuaColor::green,
		"b", &LuaColor::blue,
		"a", &LuaColor::alpha,

		// Equality comparison
		sol::meta_function::equal_to, &LuaColor::operator==,

		// String representation
		sol::meta_function::to_string, [](const LuaColor& c) {
			return "Color(" + std::to_string(c.red) + ", " +
				   std::to_string(c.green) + ", " +
				   std::to_string(c.blue) +
				   (c.alpha != 255 ? ", " + std::to_string(c.alpha) : "") + ")";
		}
	);

	// Predefined colors as constants
	sol::table colorConstants = lua.create_named_table("Colors");
	colorConstants["BLACK"] = LuaColor(0, 0, 0);
	colorConstants["WHITE"] = LuaColor(255, 255, 255);
	colorConstants["RED"] = LuaColor(255, 0, 0);
	colorConstants["GREEN"] = LuaColor(0, 255, 0);
	colorConstants["BLUE"] = LuaColor(0, 0, 255);
	colorConstants["YELLOW"] = LuaColor(255, 255, 0);
	colorConstants["CYAN"] = LuaColor(0, 255, 255);
	colorConstants["MAGENTA"] = LuaColor(255, 0, 255);
	colorConstants["ORANGE"] = LuaColor(255, 165, 0);
	colorConstants["GRAY"] = LuaColor(128, 128, 128);
	colorConstants["TRANSPARENT"] = LuaColor(0, 0, 0, 0);
}

} // namespace LuaAPI
