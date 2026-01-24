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
#include "rendering/core/text_renderer.h"
#include <nanovg.h>
#include "rendering/core/coordinate_mapper.h"
#include <wx/wx.h>

TooltipDrawer::TooltipDrawer() {
}

TooltipDrawer::~TooltipDrawer() {
	clear();
}

void TooltipDrawer::clear() {
	tooltips.clear();
}

void TooltipDrawer::addItemTooltip(const TooltipData& data) {
	if (!data.hasVisibleFields()) {
		return;
	}
	tooltips.push_back(data);
}

void TooltipDrawer::addWaypointTooltip(Position pos, const std::string& name) {
	if (name.empty()) {
		return;
	}
	TooltipData data(pos, name);
	tooltips.push_back(data);
}

void TooltipDrawer::getHeaderColor(TooltipCategory cat, uint8_t& r, uint8_t& g, uint8_t& b) const {
	using namespace TooltipColors;
	switch (cat) {
		case TooltipCategory::WAYPOINT:
			r = WAYPOINT_HEADER_R;
			g = WAYPOINT_HEADER_G;
			b = WAYPOINT_HEADER_B;
			break;
		case TooltipCategory::DOOR:
			r = DOOR_HEADER_R;
			g = DOOR_HEADER_G;
			b = DOOR_HEADER_B;
			break;
		case TooltipCategory::TELEPORT:
			r = TELEPORT_HEADER_R;
			g = TELEPORT_HEADER_G;
			b = TELEPORT_HEADER_B;
			break;
		case TooltipCategory::TEXT:
			r = TEXT_HEADER_R;
			g = TEXT_HEADER_G;
			b = TEXT_HEADER_B;
			break;
		case TooltipCategory::ITEM:
		default:
			r = ITEM_HEADER_R;
			g = ITEM_HEADER_G;
			b = ITEM_HEADER_B;
			break;
	}
}

void TooltipDrawer::draw(const RenderView& view) {
	NVGcontext* vg = TextRenderer::GetContext();
	if (!vg) {
		return;
	}

	using namespace TooltipColors;

	for (const auto& tooltip : tooltips) {
		int unscaled_x, unscaled_y;
		view.getScreenPosition(tooltip.pos.x, tooltip.pos.y, tooltip.pos.z, unscaled_x, unscaled_y);

		float zoom = view.zoom;
		if (zoom < 0.01f) {
			zoom = 1.0f;
		}

		float screen_x = unscaled_x / zoom;
		float screen_y = unscaled_y / zoom;
		float tile_size_screen = 32.0f / zoom;

		// Center on tile
		screen_x += tile_size_screen / 2.0f;
		screen_y += tile_size_screen / 2.0f;

		// Layout constants
		float fontSize = 11.0f;
		float padding = 10.0f;
		float lineHeight = fontSize * 1.4f;
		float cornerRadius = 4.0f;
		float borderWidth = 1.0f; // Thinner border
		float minWidth = 120.0f;
		float maxWidth = 220.0f; // Max content width for wrapping

		// Build content lines with word wrapping support
		struct FieldLine {
			std::string label;
			std::string value;
			uint8_t r, g, b;
			std::vector<std::string> wrappedLines; // For multi-line values
		};
		std::vector<FieldLine> fields;

		if (tooltip.category == TooltipCategory::WAYPOINT) {
			fields.push_back({ "Waypoint", tooltip.waypointName, WAYPOINT_HEADER_R, WAYPOINT_HEADER_G, WAYPOINT_HEADER_B, {} });
		} else {
			if (tooltip.actionId > 0) {
				fields.push_back({ "Action ID", std::to_string(tooltip.actionId), ACTION_ID_R, ACTION_ID_G, ACTION_ID_B, {} });
			}
			if (tooltip.uniqueId > 0) {
				fields.push_back({ "Unique ID", std::to_string(tooltip.uniqueId), UNIQUE_ID_R, UNIQUE_ID_G, UNIQUE_ID_B, {} });
			}
			if (tooltip.doorId > 0) {
				fields.push_back({ "Door ID", std::to_string(tooltip.doorId), DOOR_ID_R, DOOR_ID_G, DOOR_ID_B, {} });
			}
			if (tooltip.destination.x > 0) {
				std::string dest = std::to_string(tooltip.destination.x) + ", " + std::to_string(tooltip.destination.y) + ", " + std::to_string(tooltip.destination.z);
				fields.push_back({ "Destination", dest, TELEPORT_DEST_R, TELEPORT_DEST_G, TELEPORT_DEST_B, {} });
			}
			if (!tooltip.description.empty()) {
				fields.push_back({ "Description", tooltip.description, BODY_TEXT_R, BODY_TEXT_G, BODY_TEXT_B, {} });
			}
			if (!tooltip.text.empty()) {
				fields.push_back({ "Text", "\"" + tooltip.text + "\"", TEXT_R, TEXT_G, TEXT_B, {} });
			}
		}

		// Skip if no fields
		if (fields.empty()) {
			continue;
		}

		// Set up font for measurements
		nvgFontSize(vg, fontSize);
		nvgFontFace(vg, "sans");

		// Measure label widths and wrap long values
		float maxLabelWidth = 0.0f;
		for (auto& field : fields) {
			float labelBounds[4];
			nvgTextBounds(vg, 0, 0, field.label.c_str(), nullptr, labelBounds);
			float lw = labelBounds[2] - labelBounds[0];
			if (lw > maxLabelWidth) {
				maxLabelWidth = lw;
			}
		}

		float valueStartX = maxLabelWidth + 12.0f; // Gap between label and value
		float maxValueWidth = maxWidth - valueStartX - padding * 2;

		// Word wrap values that are too long
		int totalLines = 0;
		float actualMaxWidth = minWidth;

		for (auto& field : fields) {
			const char* start = field.value.c_str();
			const char* end = start + field.value.length();

			// Check if value fits on one line
			float valueBounds[4];
			nvgTextBounds(vg, 0, 0, start, nullptr, valueBounds);
			float valueWidth = valueBounds[2] - valueBounds[0];

			if (valueWidth <= maxValueWidth) {
				// Single line
				field.wrappedLines.push_back(field.value);
				totalLines++;
				float lineWidth = valueStartX + valueWidth + padding * 2;
				if (lineWidth > actualMaxWidth) {
					actualMaxWidth = lineWidth;
				}
			} else {
				// Need to wrap - use NanoVG text breaking
				NVGtextRow rows[16];
				int nRows = nvgTextBreakLines(vg, start, end, maxValueWidth, rows, 16);

				for (int i = 0; i < nRows; i++) {
					std::string line(rows[i].start, rows[i].end);
					field.wrappedLines.push_back(line);
					totalLines++;

					float lineWidth = valueStartX + rows[i].width + padding * 2;
					if (lineWidth > actualMaxWidth) {
						actualMaxWidth = lineWidth;
					}
				}

				if (nRows == 0) {
					// Fallback if breaking failed
					field.wrappedLines.push_back(field.value);
					totalLines++;
				}
			}
		}

		// Calculate box dimensions
		float boxWidth = std::min(maxWidth + padding * 2, std::max(minWidth, actualMaxWidth));
		float boxHeight = totalLines * lineHeight + padding * 2;

		// Position tooltip above tile
		float tooltipX = screen_x - (boxWidth / 2.0f);
		float tooltipY = screen_y - boxHeight - 12.0f;

		// Get border color based on category
		uint8_t borderR, borderG, borderB;
		getHeaderColor(tooltip.category, borderR, borderG, borderB);

		// Shadow (multi-layer soft shadow)
		for (int i = 3; i >= 0; i--) {
			float alpha = 35.0f + (3 - i) * 20.0f;
			float spread = i * 2.0f;
			float offsetY = 3.0f + i * 1.0f;
			nvgBeginPath(vg);
			nvgRoundedRect(vg, tooltipX - spread, tooltipY + offsetY - spread, boxWidth + spread * 2, boxHeight + spread * 2, cornerRadius + spread);
			nvgFillColor(vg, nvgRGBA(0, 0, 0, (int)alpha));
			nvgFill(vg);
		}

		// Main background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, tooltipX, tooltipY, boxWidth, boxHeight, cornerRadius);
		nvgFillColor(vg, nvgRGBA(BODY_BG_R, BODY_BG_G, BODY_BG_B, 250));
		nvgFill(vg);

		// Full colored border around entire frame
		nvgBeginPath(vg);
		nvgRoundedRect(vg, tooltipX, tooltipY, boxWidth, boxHeight, cornerRadius);
		nvgStrokeColor(vg, nvgRGBA(borderR, borderG, borderB, 255));
		nvgStrokeWidth(vg, borderWidth);
		nvgStroke(vg);

		// Draw content
		float contentX = tooltipX + padding;
		float cursorY = tooltipY + padding;

		nvgFontSize(vg, fontSize);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		for (const auto& field : fields) {
			bool firstLine = true;
			for (const auto& line : field.wrappedLines) {
				if (firstLine) {
					// Draw label on first line
					nvgFillColor(vg, nvgRGBA(BODY_TEXT_R, BODY_TEXT_G, BODY_TEXT_B, 160));
					nvgText(vg, contentX, cursorY, field.label.c_str(), nullptr);
					firstLine = false;
				}

				// Draw value line in semantic color
				nvgFillColor(vg, nvgRGBA(field.r, field.g, field.b, 255));
				nvgText(vg, contentX + valueStartX, cursorY, line.c_str(), nullptr);

				cursorY += lineHeight;
			}
		}
	}
}
