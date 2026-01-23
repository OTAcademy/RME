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
#include "rendering/core/batch_renderer.h"
#include "item.h"
#include "complexitem.h"
#include "waypoints.h"
#include <wx/wx.h> // For wxColor if needed, but we use OpenGL calls mostly.
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

		// TODO: Modern text rendering
		// GLUT is removed.
		width = std::max<float>(width, text ? strlen(text) * 8.0f : 0.0f);
		height = 16.0f; // Dummy height

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
		glm::vec4 bgColor(tooltip->r / 255.0f, tooltip->g / 255.0f, tooltip->b / 255.0f, 1.0f);

		// Polygon tessellation is messy. It's a speech bubble shape.
		// Convex? No, it has a tail (point 4).
		// Decompose into fan/triangles?

		// 0-1-2-3-4-5-6-7-0
		// Center 0 is not center of fan.
		// Let's just create a fan around 0? 0 is top-center.
		// Wait, 0 is {x, starty} which is top center.
		// 1 is Top Right.
		// 2 is Bottom Right (above tail).
		// 3 is Tail Start.
		// 4 is Tail Tip (y down).
		// 5 is Tail End.
		// 6 is Bottom Left.
		// 7 is Top Left.

		// Triangles:
		// 0-1-2 (Top Right Chunk? No, 0-1 and 1-2)
		// Let's use simple tessellation or GL_POLYGON emulation if BatchRenderer supported it.
		// Manual Triangles:
		// 0,1,2
		// 0,2,3
		// 0,3,6
		// 0,6,7
		// 6,3,5
		// 3,4,5 (Tail)
		// Wait, coordinates might be confusing.
		// Let's submit vertices for a polygon if logic permits, OR just assume this is a convex-ish shape + tail.
		// Actually, standard tooltip is a rounded box. This is drawing a box + tail.

		// Tessellation:
		// Quad (startx, endx, starty, endy) covers most?
		// endy < starty ? No, Y likely decreases up?
		// y = tooltip y (mouse pos).
		// starty = y - (height + space); (Above mouse)
		// So starty < y.
		// It seems Drawing is Y-up?
		// MapCanvas usually uses 0,0 top-left?
		// If 0,0 is top-left, then Y increases DOWN.
		// starty = y - big_number => Above. This implies Y increases DOWN.

		// Vertices:
		// 7(TL)---------1(TR)  <-- starty
		// |             |
		// 6(BL)---5-3---2(BR)  <-- endy
		//          \ /
		//           4(Tip)     <-- y

		// So we have a Box (1,2,6,7) and a Triangle (3,4,5).
		// Box: (startx, starty) to (endx, endy).
		BatchRenderer::DrawQuad(
			glm::vec2(startx, starty),
			glm::vec2(width, height),
			bgColor
		);

		// Tail Triangle (3,4,5)
		BatchRenderer::DrawTriangle(
			glm::vec2(x + space, endy), // 3
			glm::vec2(x, y), // 4
			glm::vec2(x - space, endy), // 5
			bgColor
		);

		// borders
		glm::vec4 borderColor(0.0f, 0.0f, 0.0f, 1.0f);
		for (int i = 0; i < 8; ++i) {
			BatchRenderer::DrawLine(
				glm::vec2(vertexes[i][0], vertexes[i][1]),
				glm::vec2(vertexes[i + 1][0], vertexes[i + 1][1]),
				borderColor
			);
		}

		// text
		// Immediate mode GLUT text cannot be batched easily.
		// We Must Flush before drawing direct GL/GLUT text.
		BatchRenderer::Flush();

		if (zoom <= 1.0) {
			startx += (3.0f * scale);
			starty += (14.0f * scale);

			// TODO: Implement text rendering using a font atlas/BatchRenderer.
			// Legacy GLUT/glBitmap text is not supported in Core Profile and has been removed.
		}
	}
}
