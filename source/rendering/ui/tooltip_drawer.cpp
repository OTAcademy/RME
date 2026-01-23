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
#include "rendering/core/text_renderer.h"
#include <nanovg.h>
#include "rendering/core/coordinate_mapper.h"
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

void TooltipDrawer::addTooltip(Position pos, const std::string& text, uint8_t r, uint8_t g, uint8_t b) {
	if (text.empty()) {
		return;
	}

	MapTooltip* tooltip = newd MapTooltip(pos, text, r, g, b);
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

void TooltipDrawer::draw(const RenderView& view) {
	NVGcontext* vg = TextRenderer::GetContext();
	if (!vg) {
		return;
	}

	for (std::vector<MapTooltip*>::const_iterator it = tooltips.begin(); it != tooltips.end(); ++it) {
		MapTooltip* tooltip = (*it);
		const std::string& text = tooltip->text;
		if (text.empty()) {
			continue;
		}

		int unscaled_x, unscaled_y;
		view.getScreenPosition(tooltip->pos.x, tooltip->pos.y, tooltip->pos.z, unscaled_x, unscaled_y);

		// Convert to Screen Pixels (NanoVG Space)
		// The map is rendered with glOrtho(0, width*zoom, height*zoom, 0, ...)
		// This means 1 Logical Unit = (1 / zoom) Screen Pixels.
		float zoom = view.zoom;
		if (zoom < 0.01f) {
			zoom = 1.0f; // Safety
		}

		float screen_x = unscaled_x / zoom;
		float screen_y = unscaled_y / zoom;
		float tile_size_screen = 32.0f / zoom;

		// Adjust to center of tile
		screen_x += tile_size_screen / 2.0f;
		screen_y += tile_size_screen / 2.0f;

		// --- Text Layout (Multiline) ---
		float fontSize = 12.0f; // Consistent small font
		nvgFontSize(vg, fontSize);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		// Process lines
		float maxLineWidth = 250.0f; // Tibia-like max width
		float lineHeight = fontSize * 1.2f;

		struct TextRow {
			std::string content;
			float width;
		};
		std::vector<TextRow> rows;
		float totalTextWidth = 0.0f;

		// 1. Split by \n
		std::stringstream ss(text);
		std::string segment;
		while (std::getline(ss, segment, '\n')) {
			if (segment.empty()) {
				// Empty line (optional: add spacing or ignore)
				continue;
			}

			// 2. Wrap long lines using NanoVG
			const char* start = segment.c_str();
			const char* end = start + segment.length();
			NVGtextRow nvgRows[32]; // Max 32 lines per segment
			int nRows = nvgTextBreakLines(vg, start, end, maxLineWidth, nvgRows, 32);

			for (int i = 0; i < nRows; i++) {
				std::string lineStr(nvgRows[i].start, nvgRows[i].end);
				float w = nvgRows[i].width; // Bounds of this line
				// nvgTextBreakLines returns width, or we can measure it
				// float b[4]; nvgTextBounds(vg, 0,0, lineStr.c_str(), nullptr, b); w = b[2]-b[0];

				if (w > totalTextWidth) {
					totalTextWidth = w;
				}
				rows.push_back({ lineStr, w });
			}
		}

		if (rows.empty()) {
			continue;
		}

		float padding = 6.0f;
		float boxWidth = totalTextWidth + (padding * 2);
		float boxHeight = (rows.size() * lineHeight) + (padding * 2);
		float cornerRadius = 4.0f;

		// Position tooltip above the tile (centered horizontally)
		float tooltipX = screen_x - (boxWidth / 2.0f);
		float tooltipY = screen_y - boxHeight - 8.0f;

		// --- Minimalistic Floating Tooltip ---

		// Floating Shadow (soft, offset for 3D effect)
		float shadowOffsetX = 4.0f;
		float shadowOffsetY = 6.0f;

		// Draw soft shadow using multiple layers for blur effect
		for (int i = 3; i >= 0; i--) {
			float alpha = 30.0f + (3 - i) * 20.0f; // Gradient opacity
			float spread = i * 2.0f;
			nvgBeginPath(vg);
			nvgRoundedRect(vg, tooltipX + shadowOffsetX - spread, tooltipY + shadowOffsetY - spread, boxWidth + spread * 2, boxHeight + spread * 2, cornerRadius + spread);
			nvgFillColor(vg, nvgRGBA(0, 0, 0, (int)alpha));
			nvgFill(vg);
		}

		// Main Background (clean, slightly warm white)
		nvgBeginPath(vg);
		nvgRoundedRect(vg, tooltipX, tooltipY, boxWidth, boxHeight, cornerRadius);
		nvgFillColor(vg, nvgRGBA(250, 248, 245, 255)); // Off-white / warm
		nvgFill(vg);

		// Subtle Border
		nvgBeginPath(vg);
		nvgRoundedRect(vg, tooltipX + 0.5f, tooltipY + 0.5f, boxWidth - 1, boxHeight - 1, cornerRadius);
		nvgStrokeColor(vg, nvgRGBA(180, 170, 160, 255)); // Light brown/grey
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);

		// Text (dark for contrast)
		nvgFillColor(vg, nvgRGBA(40, 35, 30, 255));
		float cursorY = tooltipY + padding;
		for (const auto& row : rows) {
			nvgText(vg, tooltipX + padding, cursorY, row.content.c_str(), nullptr);
			cursorY += lineHeight;
		}
	}
}
