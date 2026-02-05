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

#include "app/definitions.h"
#include <iostream>
#include "map/position.h"
#include "rendering/core/render_view.h"
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <unordered_map>

class Item;
class Waypoint;
struct NVGcontext;

struct ContainerItem {
	uint16_t id;
	uint8_t count;
	uint8_t subtype;
};

// Tooltip category determines header color and icon
enum class TooltipCategory {
	WAYPOINT, // Green - landmark
	ITEM, // Dark charcoal - generic item
	DOOR, // Brown - door with door ID
	TELEPORT, // Purple - teleporter
	TEXT // Gold - readable text (signs, books)
};

// Semantic color palette for tooltip rendering
namespace TooltipColors {
	// Header colors by category (RGB)
	constexpr uint8_t WAYPOINT_HEADER_R = 50, WAYPOINT_HEADER_G = 205, WAYPOINT_HEADER_B = 50; // Lime Green
	constexpr uint8_t ITEM_HEADER_R = 58, ITEM_HEADER_G = 58, ITEM_HEADER_B = 64; // Charcoal
	constexpr uint8_t DOOR_HEADER_R = 139, DOOR_HEADER_G = 69, DOOR_HEADER_B = 19; // Saddle Brown
	constexpr uint8_t TELEPORT_HEADER_R = 153, TELEPORT_HEADER_G = 50, TELEPORT_HEADER_B = 204; // Dark Orchid
	constexpr uint8_t TEXT_HEADER_R = 218, TEXT_HEADER_G = 165, TEXT_HEADER_B = 32; // Goldenrod

	// Field value colors (RGB)
	constexpr uint8_t ACTION_ID_R = 255, ACTION_ID_G = 165, ACTION_ID_B = 0; // Orange
	constexpr uint8_t UNIQUE_ID_R = 100, UNIQUE_ID_G = 149, UNIQUE_ID_B = 237; // Cornflower Blue
	constexpr uint8_t DOOR_ID_R = 0, DOOR_ID_G = 139, DOOR_ID_B = 139; // Dark Cyan
	constexpr uint8_t TEXT_R = 255, TEXT_G = 215, TEXT_B = 0; // Gold
	constexpr uint8_t TELEPORT_DEST_R = 186, TELEPORT_DEST_G = 85, TELEPORT_DEST_B = 211; // Medium Orchid

	// Body colors
	constexpr uint8_t BODY_BG_R = 30, BODY_BG_G = 30, BODY_BG_B = 35; // Near Black
	constexpr uint8_t BODY_TEXT_R = 220, BODY_TEXT_G = 220, BODY_TEXT_B = 220; // Light Gray
	constexpr uint8_t HEADER_TEXT_R = 255, HEADER_TEXT_G = 255, HEADER_TEXT_B = 255; // White
	constexpr uint8_t COUNT_TEXT_R = 200, COUNT_TEXT_G = 200, COUNT_TEXT_B = 200; // Light Gray for counts
}

// Structured tooltip data for card-based rendering
struct TooltipData {
	Position pos;
	TooltipCategory category = TooltipCategory::ITEM;

	// Header info
	uint16_t itemId = 0;
	std::string_view itemName;

	// Optional fields (0 or empty = not shown)
	uint16_t actionId = 0;
	uint16_t uniqueId = 0;
	uint8_t doorId = 0;
	std::string_view text;
	std::string_view description;
	Position destination; // For teleports (check if valid via destination.x > 0)

	// Waypoint-specific
	std::string_view waypointName;

	// Container contents
	std::vector<ContainerItem> containerItems;
	uint8_t containerCapacity = 0;

	TooltipData() = default;

	// Constructor for waypoint
	TooltipData(Position p, std::string_view wpName) :
		pos(p), category(TooltipCategory::WAYPOINT), waypointName(wpName) { }

	// Constructor for item
	TooltipData(Position p, uint16_t id, std::string_view name) :
		pos(p), category(TooltipCategory::ITEM), itemId(id), itemName(name) { }

	// Determine category based on fields
	void updateCategory() {
		if (!waypointName.empty()) {
			category = TooltipCategory::WAYPOINT;
		} else if (destination.x > 0) {
			category = TooltipCategory::TELEPORT;
		} else if (doorId > 0) {
			category = TooltipCategory::DOOR;
		} else if (!text.empty()) {
			category = TooltipCategory::TEXT;
		} else {
			category = TooltipCategory::ITEM;
		}
	}

	// Check if this tooltip has any visible fields
	bool hasVisibleFields() const {
		return !waypointName.empty() || actionId > 0 || uniqueId > 0 || doorId > 0 || !text.empty() || !description.empty() || destination.x > 0 || !containerItems.empty();
	}

	void clear() {
		// Reset scalars
		pos = Position();
		category = TooltipCategory::ITEM;
		itemId = 0;
		// clear strings
		itemName = {};
		actionId = 0;
		uniqueId = 0;
		doorId = 0;
		text = {};
		description = {};
		destination = Position();
		waypointName = {};
		containerItems.clear();
		containerCapacity = 0;
	}
};

class TooltipDrawer {
public:
	TooltipDrawer();
	~TooltipDrawer();

	// Add a structured tooltip for an item
	void addItemTooltip(const TooltipData& data);
	void addItemTooltip(TooltipData&& data);

	// Request a tooltip object from the pool. Call commitTooltip() to finalize.
	TooltipData& requestTooltipData();
	void commitTooltip();

	// Add a waypoint tooltip
	void addWaypointTooltip(Position pos, std::string_view name);

	// Draw all tooltips
	void draw(NVGcontext* vg, const RenderView& view);

	// Clear all tooltips
	void clear();

protected:
	struct FieldLine {
		std::string label;
		std::string value;
		uint8_t r, g, b;
		std::vector<std::string> wrappedLines; // For multi-line values
	};
	std::vector<FieldLine> scratch_fields;

	std::vector<TooltipData> tooltips;
	size_t active_count = 0;

	std::unordered_map<uint32_t, int> spriteCache; // sprite_id -> nvg image handle
	NVGcontext* lastContext = nullptr;

	// Helper to get or load sprite image
	int getSpriteImage(NVGcontext* vg, uint16_t itemId);

	// Helper to get header color based on category
	void getHeaderColor(TooltipCategory cat, uint8_t& r, uint8_t& g, uint8_t& b) const;

	// Refactored drawing helpers
	struct LayoutMetrics {
		float width;
		float height;
		float valueStartX;
		float gridSlotSize;
		int containerCols;
		int containerRows;
		float containerHeight;
		int totalContainerSlots;
		int emptyContainerSlots;
		int numContainerItems;
	};

	void prepareFields(const TooltipData& tooltip);
	LayoutMetrics calculateLayout(NVGcontext* vg, const TooltipData& tooltip, float maxWidth, float minWidth, float padding, float fontSize);
	void drawBackground(NVGcontext* vg, float x, float y, float width, float height, float cornerRadius, const TooltipData& tooltip);
	void drawFields(NVGcontext* vg, float x, float y, float valueStartX, float lineHeight, float padding, float fontSize);
	void drawContainerGrid(NVGcontext* vg, float x, float y, const TooltipData& tooltip, const LayoutMetrics& layout);
};

#endif
