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

#include "rendering/ui/tooltip_drawer.h"
#include "rendering/core/graphics.h"
#include "item.h"
#include "complexitem.h"
#include "waypoints.h"
#include <wx/wx.h> // For wxColor if needed, but we use OpenGL calls mostly.
#include <GL/glut.h>
#include "common.h" // for newd macro

TooltipDrawer::TooltipDrawer() {
}

TooltipDrawer::~TooltipDrawer() {
	clear();
}

void TooltipDrawer::clear() {
	for (auto* tooltip : tooltips) {
		delete tooltip;
	}
	tooltips.clear();
}

void TooltipDrawer::addTooltip(int screenx, int screeny, const std::string& text, uint8_t r, uint8_t g, uint8_t b) {
	if (text.empty()) {
		return;
	}

	MapTooltip* tooltip = newd MapTooltip(screenx, screeny, text, r, g, b);
	tooltip->checkLineEnding();
	tooltips.push_back(tooltip);
}

void TooltipDrawer::writeTooltip(Item* item, std::ostringstream& stream, bool isHouseTile) {
	if (item == nullptr) {
		return;
	}

	const uint16_t id = item->getID();
	if (id < 100) {
		return;
	}

	const uint16_t unique = item->getUniqueID();
	const uint16_t action = item->getActionID();
	const std::string& text = item->getText();
	uint8_t doorId = 0;

	if (isHouseTile && item->isDoor()) {
		if (Door* door = dynamic_cast<Door*>(item)) {
			if (door->isRealDoor()) {
				doorId = door->getDoorID();
			}
		}
	}

	Teleport* tp = dynamic_cast<Teleport*>(item);
	if (unique == 0 && action == 0 && doorId == 0 && text.empty() && !tp) {
		return;
	}

	if (stream.tellp() > 0) {
		stream << "\n";
	}

	stream << "id: " << id << "\n";

	if (action > 0) {
		stream << "aid: " << action << "\n";
	}
	if (unique > 0) {
		stream << "uid: " << unique << "\n";
	}
	if (doorId > 0) {
		stream << "door id: " << static_cast<int>(doorId) << "\n";
	}
	if (!text.empty()) {
		stream << "text: " << text << "\n";
	}
	if (tp) {
		const Position& dest = tp->getDestination();
		stream << "destination: " << dest.x << ", " << dest.y << ", " << dest.z << "\n";
	}
}

void TooltipDrawer::writeTooltip(Waypoint* waypoint, std::ostringstream& stream) {
	if (stream.tellp() > 0) {
		stream << "\n";
	}
	stream << "wp: " << waypoint->name << "\n";
}

void TooltipDrawer::draw(float zoom, int tile_size) {
	for (std::vector<MapTooltip*>::const_iterator it = tooltips.begin(); it != tooltips.end(); ++it) {
		MapTooltip* tooltip = (*it);
		const char* text = tooltip->text.c_str();
		float line_width = 0.0f;
		float width = 2.0f;
		float height = 14.0f;
		int char_count = 0;
		int line_char_count = 0;

		for (const char* c = text; *c != '\0'; c++) {
			if (*c == '\n' || (line_char_count >= MapTooltip::MAX_CHARS_PER_LINE && *c == ' ')) {
				height += 14.0f;
				line_width = 0.0f;
				line_char_count = 0;
			} else {
				line_width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
			}
			width = std::max<float>(width, line_width);
			char_count++;
			line_char_count++;

			if (tooltip->ellipsis && char_count > (MapTooltip::MAX_CHARS + 3)) {
				break;
			}
		}

		float scale = zoom < 1.0f ? zoom : 1.0f;

		width = (width + 8.0f) * scale;
		height = (height + 4.0f) * scale;

		float x = tooltip->x + (tile_size / 2.0f);
		float y = tooltip->y;
		float center = width / 2.0f;
		float space = (7.0f * scale);
		float startx = x - center;
		float endx = x + center;
		float starty = y - (height + space);
		float endy = y - space;

		// 7----0----1
		// |         |
		// 6--5  3--2
		//     \/
		//     4
		float vertexes[9][2] = {
			{ x, starty }, // 0
			{ endx, starty }, // 1
			{ endx, endy }, // 2
			{ x + space, endy }, // 3
			{ x, y }, // 4
			{ x - space, endy }, // 5
			{ startx, endy }, // 6
			{ startx, starty }, // 7
			{ x, starty }, // 0
		};

		// background
		glColor4ub(tooltip->r, tooltip->g, tooltip->b, 255);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 8; ++i) {
			glVertex2f(vertexes[i][0], vertexes[i][1]);
		}
		glEnd();

		// borders
		glColor4ub(0, 0, 0, 255);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		for (int i = 0; i < 8; ++i) {
			glVertex2f(vertexes[i][0], vertexes[i][1]);
			glVertex2f(vertexes[i + 1][0], vertexes[i + 1][1]);
		}
		glEnd();

		// text
		if (zoom <= 1.0) {
			startx += (3.0f * scale);
			starty += (14.0f * scale);
			glColor4ub(0, 0, 0, 255);
			glRasterPos2f(startx, starty);
			char_count = 0;
			line_char_count = 0;
			for (const char* c = text; *c != '\0'; c++) {
				if (*c == '\n' || (line_char_count >= MapTooltip::MAX_CHARS_PER_LINE && *c == ' ')) {
					starty += (14.0f * scale);
					glRasterPos2f(startx, starty);
					line_char_count = 0;
				}
				char_count++;
				line_char_count++;

				if (tooltip->ellipsis && char_count >= MapTooltip::MAX_CHARS) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '.');
					if (char_count >= (MapTooltip::MAX_CHARS + 2)) {
						break;
					}
				} else if (!iscntrl(*c)) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
				}
			}
		}
	}
}
