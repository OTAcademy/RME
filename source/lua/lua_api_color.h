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

#ifndef RME_LUA_API_COLOR_H
#define RME_LUA_API_COLOR_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace LuaAPI {

// Simple RGBA color class for Lua scripting
struct LuaColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	LuaColor() : red(0), green(0), blue(0), alpha(255) {}
	LuaColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
		: red(r), green(g), blue(b), alpha(a) {}

	// Convert to wxColour for use with wxWidgets
	wxColour toWxColour() const {
		return wxColour(red, green, blue, alpha);
	}

	// Create from wxColour
	static LuaColor fromWxColour(const wxColour& c) {
		return LuaColor(c.Red(), c.Green(), c.Blue(), c.Alpha());
	}

	// Equality comparison
	bool operator==(const LuaColor& other) const {
		return red == other.red && green == other.green &&
			   blue == other.blue && alpha == other.alpha;
	}
};

// Register the Color usertype with Lua
void registerColor(sol::state& lua);

} // namespace LuaAPI

#endif // RME_LUA_API_COLOR_H
