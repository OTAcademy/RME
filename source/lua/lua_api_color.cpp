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
		sol::table Color = lua.create_table();

		// Constructor rgb
		Color["rgb"] = [&lua](int r, int g, int b) {
			sol::table c = lua.create_table();
			c["r"] = r;
			c["g"] = g;
			c["b"] = b;
			return c;
		};

		// Constructor hex
		Color["hex"] = [&lua](const std::string& hex) {
			unsigned long value = 0;
			std::string h = hex[0] == '#' ? hex.substr(1) : hex;
			if (h.length() == 3) {
				h = {h[0], h[0], h[1], h[1], h[2], h[2]};
			}
			try {
				value = std::stoul(h, nullptr, 16);
			} catch (...) {
				value = 0;
			}

			sol::table c = lua.create_table();
			c["r"] = (int)((value >> 16) & 0xFF);
			c["g"] = (int)((value >> 8) & 0xFF);
			c["b"] = (int)(value & 0xFF);
			return c;
		};

		// Lighten/Darken helper using wxColour
		Color["lighten"] = [&lua](sol::table c, int percent) {
			wxColour wx(c.get_or("r", 0), c.get_or("g", 0), c.get_or("b", 0));
			wxColour result = wx.ChangeLightness(100 + percent);

			sol::table res = lua.create_table();
			res["r"] = (int)result.Red();
			res["g"] = (int)result.Green();
			res["b"] = (int)result.Blue();
			return res;
		};

		Color["darken"] = [&lua](sol::table c, int percent) {
			wxColour wx(c.get_or("r", 0), c.get_or("g", 0), c.get_or("b", 0));
			wxColour result = wx.ChangeLightness(100 - percent);

			sol::table res = lua.create_table();
			res["r"] = (int)result.Red();
			res["g"] = (int)result.Green();
			res["b"] = (int)result.Blue();
			return res;
		};

		// Helper to create a color table
		auto mkColor = [&lua](int r, int g, int b) {
			sol::table c = lua.create_table();
			c["r"] = r;
			c["g"] = g;
			c["b"] = b;
			return c;
		};

		// Predefined colors
		Color["white"] = mkColor(255, 255, 255);
		Color["black"] = mkColor(0, 0, 0);
		Color["blue"] = mkColor(49, 130, 206);
		Color["red"] = mkColor(220, 53, 69);
		Color["green"] = mkColor(40, 167, 69);
		Color["yellow"] = mkColor(255, 193, 7);
		Color["orange"] = mkColor(253, 126, 20);
		Color["gray"] = mkColor(128, 128, 128);
		Color["lightGray"] = mkColor(245, 247, 250);
		Color["darkGray"] = mkColor(45, 55, 72);

		lua["Color"] = Color;
	}

} // namespace LuaAPI
