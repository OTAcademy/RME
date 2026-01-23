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

#ifndef RME_TOOLTIP_DRAWER_H_
#define RME_TOOLTIP_DRAWER_H_

#include "../../definitions.h"
#include <iostream>
#include "position.h"
#include "rendering/core/render_view.h"
#include <vector>
#include <string>
#include <sstream>

class Item;
class Waypoint;

struct MapTooltip {
	enum TextLength {
		MAX_CHARS_PER_LINE = 40,
		MAX_CHARS = 255,
	};

	MapTooltip(Position pos, std::string text, uint8_t r, uint8_t g, uint8_t b) :
		pos(pos), text(text), r(r), g(g), b(b) {
		ellipsis = (text.length() - 3) > MAX_CHARS;
	}

	void checkLineEnding() {
		if (text.at(text.size() - 1) == '\n') {
			text.resize(text.size() - 1);
		}
	}

	Position pos;
	std::string text;
	uint8_t r, g, b;
	bool ellipsis;
};

class TooltipDrawer {
public:
	TooltipDrawer();
	~TooltipDrawer();

	void addTooltip(Position pos, const std::string& text, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);
	void writeTooltip(Item* item, std::ostringstream& stream, bool isHouseTile = false);
	void writeTooltip(Waypoint* waypoint, std::ostringstream& stream);
	void draw(const RenderView& view);
	void clear();

protected:
	std::vector<MapTooltip*> tooltips;
};

#endif
