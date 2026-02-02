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
#include "game/items.h"
#include "game/sprites.h"
#include "ui/gui.h"

TooltipDrawer::TooltipDrawer() {
}

TooltipDrawer::~TooltipDrawer() {
	clear();
	if (lastContext) {
		for (auto& pair : spriteCache) {
			if (pair.second > 0) {
				nvgDeleteImage(lastContext, pair.second);
			}
		}
		spriteCache.clear();
	}
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

int TooltipDrawer::getSpriteImage(NVGcontext* vg, uint16_t itemId) {
	if (itemId == 0) {
		return 0;
	}

	// Detect context change and clear cache
	if (vg != lastContext) {
		// If we had a previous context, we'd ideally delete images from it,
		// but if the context pointer changed, the old one might be invalid.
		// However, adhering to the request to clear cache with nvgDeleteImage:
		if (lastContext) {
			for (auto& pair : spriteCache) {
				if (pair.second > 0) {
					nvgDeleteImage(lastContext, pair.second);
				}
			}
		}
		spriteCache.clear();
		lastContext = vg;
	}

	// Resolve Item ID
	ItemType& it = g_items[itemId];
	if (!it.sprite) {
		return 0;
	}

	// We use the item ID as the cache key since it's unique and stable
	auto itCache = spriteCache.find(itemId);
	if (itCache != spriteCache.end()) {
		return itCache->second;
	}

	// Use the sprite directly from ItemType
	GameSprite* gameSprite = it.sprite;

	if (gameSprite && !gameSprite->spriteList.empty()) {
		// Use the first frame/part of the sprite
		GameSprite::NormalImage* img = gameSprite->spriteList[0];
		if (img) {
			std::unique_ptr<uint8_t[]> rgba;

			// For legacy sprites (no transparency), use getRGBData + Magenta Masking
			// This matches how WxWidgets/SpriteIconGenerator renders icons
			if (!g_gui.gfx.hasTransparency()) {
				std::unique_ptr<uint8_t[]> rgb = img->getRGBData();
				if (rgb) {
					rgba = std::make_unique<uint8_t[]>(32 * 32 * 4);
					for (int i = 0; i < 32 * 32; ++i) {
						uint8_t r = rgb[i * 3 + 0];
						uint8_t g = rgb[i * 3 + 1];
						uint8_t b = rgb[i * 3 + 2];

						// Magic Pink (Magenta) is transparent for legacy sprites
						if (r == 0xFF && g == 0x00 && b == 0xFF) {
							rgba[i * 4 + 0] = 0;
							rgba[i * 4 + 1] = 0;
							rgba[i * 4 + 2] = 0;
							rgba[i * 4 + 3] = 0;
						} else {
							rgba[i * 4 + 0] = r;
							rgba[i * 4 + 1] = g;
							rgba[i * 4 + 2] = b;
							rgba[i * 4 + 3] = 255;
						}
					}
				} else {
					// getRGBData failed, should ideally not happen if sprite exists logic is correct
				}
			}

			// Fallback/Standard path for alpha sprites or if RGB failed
			if (!rgba) {
				rgba = img->getRGBAData();
				if (!rgba) {
					// getRGBAData failed
				}
			}

			if (rgba) {
				int image = nvgCreateImageRGBA(vg, 32, 32, 0, rgba.get());
				if (image == 0) {
					// nvgCreateImageRGBA failed
				} else {
					// Success
					// Check if we are overwriting an existing valid image (shouldn't happen given find() above, but safest)
					if (spriteCache.count(itemId) && spriteCache[itemId] > 0) {
						nvgDeleteImage(vg, spriteCache[itemId]);
					}
					spriteCache[itemId] = image;
					return image;
				}
			}
		}
	} else {
		// GameSprite missing or empty list
	}

	return 0;
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

		// Skip if no fields and no container items
		if (fields.empty() && tooltip.containerItems.empty()) {
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

		// Calculate container grid dimensions
		int containerCols = 0;
		int containerRows = 0;
		float gridSlotSize = 34.0f; // 32px + padding
		float containerHeight = 0.0f;

		int numItems = (int)tooltip.containerItems.size();
		int capacity = (int)tooltip.containerCapacity;
		int emptySlots = std::max(0, capacity - numItems);
		int totalSlots = numItems;

		if (emptySlots > 0) {
			totalSlots++; // Add one slot for the summary
		}

		// Apply a hard cap for visual safety (though items are capped at 32 in TileRenderer)
		if (totalSlots > 33) {
			totalSlots = 33;
		}

		if (capacity > 0 || numItems > 0) {
			// Heuristic: try to keep it somewhat square but matching width
			containerCols = std::min(4, totalSlots);
			// Force roughly square-ish if many items, but max 4-5 cols
			if (totalSlots > 4) {
				containerCols = 5;
			}
			if (totalSlots > 10) {
				containerCols = 6;
			}
			if (totalSlots > 15) {
				containerCols = 8;
			}

			// Ensure at least 1 col if there are slots
			if (containerCols == 0 && totalSlots > 0) {
				containerCols = 1;
			}

			if (containerCols > 0) {
				containerRows = (totalSlots + containerCols - 1) / containerCols;
				containerHeight = containerRows * gridSlotSize + 4.0f; // + top margin
			}
		}

		// Calculate box dimensions
		float boxWidth = std::min(maxWidth + padding * 2, std::max(minWidth, actualMaxWidth));
		// Expand max width if grid needs it (override clamp)
		bool hasContainer = totalSlots > 0;
		if (hasContainer) {
			float gridWidth = containerCols * gridSlotSize;
			boxWidth = std::max(boxWidth, gridWidth + padding * 2);
		}

		float boxHeight = totalLines * lineHeight + padding * 2;
		if (hasContainer) {
			boxHeight += containerHeight + 4.0f; // Add grid area
		}

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

		// Draw container items
		if (totalSlots > 0) {
			cursorY += 8.0f; // Spacer

			float startX = contentX;
			float startY = cursorY;

			for (int idx = 0; idx < totalSlots; ++idx) {
				int col = idx % containerCols;
				int row = idx / containerCols;

				float itemX = startX + col * gridSlotSize;
				float itemY = startY + row * gridSlotSize;

				// Draw slot background (always)
				nvgBeginPath(vg);
				nvgRect(vg, itemX, itemY, 32, 32);
				nvgFillColor(vg, nvgRGBA(60, 60, 60, 100)); // Dark slot placeholder
				nvgStrokeColor(vg, nvgRGBA(100, 100, 100, 100)); // Light border
				nvgStrokeWidth(vg, 1.0f);
				nvgFill(vg);
				nvgStroke(vg);

				// Check if this is the summary info slot (last slot if we have empty spaces)
				bool isSummarySlot = (emptySlots > 0 && idx == totalSlots - 1);

				if (isSummarySlot) {
					// Draw empty slots count: "+N"
					std::string summary = "+" + std::to_string(emptySlots);

					nvgFontSize(vg, 12.0f);
					nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

					// Text shadow
					nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
					nvgText(vg, itemX + 17, itemY + 17, summary.c_str(), nullptr); // +1 offset

					// Text
					nvgFillColor(vg, nvgRGBA(COUNT_TEXT_R, COUNT_TEXT_G, COUNT_TEXT_B, 255));
					nvgText(vg, itemX + 16, itemY + 16, summary.c_str(), nullptr);

				} else if (idx < numItems) {
					// Draw Actual Item
					const auto& item = tooltip.containerItems[idx];

					// Draw item sprite
					int img = getSpriteImage(vg, item.id);
					if (img > 0) {
						nvgBeginPath(vg);
						nvgRect(vg, itemX, itemY, 32, 32);
						nvgFillPaint(vg, nvgImagePattern(vg, itemX, itemY, 32, 32, 0, img, 1.0f));
						nvgFill(vg);
					}
					// Else: already drew placeholder background

					// Draw Count
					if (item.count > 1) {
						std::string countStr = std::to_string(item.count);
						nvgFontSize(vg, 10.0f);
						nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);

						// Text shadow
						nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
						nvgText(vg, itemX + 33, itemY + 33, countStr.c_str(), nullptr);

						// Text
						nvgFillColor(vg, nvgRGBA(COUNT_TEXT_R, COUNT_TEXT_G, COUNT_TEXT_B, 255));
						nvgText(vg, itemX + 32, itemY + 32, countStr.c_str(), nullptr);
					}
				}
			}
		}
	}
}
