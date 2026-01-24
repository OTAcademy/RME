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
#include "lua_api_item.h"
#include "../item.h"
#include "../items.h"

#include <algorithm>
#include <cctype>

namespace LuaAPI {

	// Helper: convert string to lowercase
	static std::string toLower(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
		return result;
	}

	// Helper: check if string contains another (case-insensitive)
	static bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
		std::string lowerHaystack = toLower(haystack);
		std::string lowerNeedle = toLower(needle);
		return lowerHaystack.find(lowerNeedle) != std::string::npos;
	}

	void registerItem(sol::state& lua) {
		// Register Item usertype
		lua.new_usertype<Item>(
			"Item",
			// No public constructor - items are created via Tile:addItem() or obtained from tiles
			sol::no_constructor,

			// Read-only properties
			"id", sol::property(&Item::getID),
			"clientId", sol::property(&Item::getClientID),
			"name", sol::property(&Item::getName),
			"fullName", sol::property(&Item::getFullName),

			// Read/write properties
			"count", sol::property(&Item::getCount, [](Item& item, int count) {
				item.setSubtype(static_cast<uint16_t>(count));
			}),
			"subtype", sol::property([](const Item& item) -> int { return item.getSubtype(); }, [](Item& item, int subtype) { item.setSubtype(static_cast<uint16_t>(subtype)); }), "actionId", sol::property([](const Item& item) -> int { return item.getActionID(); }, [](Item& item, int aid) { item.setActionID(static_cast<uint16_t>(aid)); }), "uniqueId", sol::property([](const Item& item) -> int { return item.getUniqueID(); }, [](Item& item, int uid) { item.setUniqueID(static_cast<uint16_t>(uid)); }), "tier", sol::property([](const Item& item) -> int { return item.getTier(); }, [](Item& item, int tier) { item.setTier(static_cast<uint16_t>(tier)); }), "text", sol::property(&Item::getText, &Item::setText), "description", sol::property(&Item::getDescription, &Item::setDescription),

			// Selection
			"isSelected", sol::property(&Item::isSelected), "select", &Item::select, "deselect", &Item::deselect,

			// Type checks (read-only)
			"isStackable", sol::property(&Item::isStackable), "isMoveable", sol::property(&Item::isMoveable), "isPickupable", sol::property(&Item::isPickupable), "isBlocking", sol::property(&Item::isBlocking), "isGroundTile", sol::property(&Item::isGroundTile), "isBorder", sol::property(&Item::isBorder), "isWall", sol::property(&Item::isWall), "isDoor", sol::property(&Item::isDoor), "isTable", sol::property(&Item::isTable), "isCarpet", sol::property(&Item::isCarpet), "isHangable", sol::property(&Item::isHangable), "isRoteable", sol::property(&Item::isRoteable), "isFluidContainer", sol::property(&Item::isFluidContainer), "isSplash", sol::property(&Item::isSplash), "hasCharges", sol::property(&Item::hasCharges), "hasElevation", sol::property([](const Item& item) {
				return g_items[item.getID()].hasElevation;
			}),
			"zOrder", sol::property(&Item::getTopOrder),

			// Methods
			"clone", [](const Item& item) -> Item* { return item.deepCopy(); }, "rotate", &Item::doRotate,

			"getName", [](int id) -> std::string {
			if (g_items.typeExists(id)) {
				return g_items.getItemType(id).name;
			}
			return ""; }, "getDescription", [](int id) -> std::string {
			if (g_items.typeExists(id)) {
				return g_items.getItemType(id).description;
			}
			return ""; },

			// String representation
			sol::meta_function::to_string, [](const Item& item) { return "Item(id=" + std::to_string(item.getID()) + ", name=\"" + item.getName() + "\")"; }
		);

		// Register Items namespace for item lookup
		sol::table items = lua.create_named_table("Items");

		// Get item type info by ID
		items["get"] = [](int id) -> sol::optional<sol::table> {
			if (!g_items.typeExists(id)) {
				return sol::nullopt;
			}
			// Return item info as table (not the actual ItemType pointer)
			return sol::nullopt; // We'll use getInfo instead
		};

		// Get item info by ID - returns a table with item properties
		items["getInfo"] = [](sol::this_state ts, int id) -> sol::object {
			sol::state_view lua(ts);
			if (!g_items.typeExists(id)) {
				return sol::nil;
			}
			ItemType& it = g_items.getItemType(id);
			sol::table info = lua.create_table();
			info["id"] = it.id;
			info["clientId"] = it.clientID;
			info["name"] = it.name;
			info["description"] = it.description;
			info["isStackable"] = it.stackable;
			info["isMoveable"] = it.moveable;
			info["isPickupable"] = it.pickupable;
			info["isGroundTile"] = it.isGroundTile();
			info["isBorder"] = it.isBorder;
			info["isWall"] = it.isWall;
			info["isDoor"] = it.isDoor();
			info["isTable"] = it.isTable;
			info["isCarpet"] = it.isCarpet;
			info["hasElevation"] = it.hasElevation;
			return info;
		};

		// Check if item ID exists
		items["exists"] = [](int id) -> bool {
			return g_items.typeExists(id);
		};

		// Get max item ID
		items["getMaxId"] = []() -> int {
			return g_items.getMaxID();
		};

		// Find items by name (returns array of {id, name} tables)
		// Searches for items whose name contains the search string (case-insensitive)
		items["findByName"] = [](sol::this_state ts, const std::string& searchName, sol::optional<int> maxResults) -> sol::table {
			sol::state_view lua(ts);
			sol::table results = lua.create_table();

			int limit = maxResults.value_or(50); // Default max 50 results
			int count = 0;
			int maxId = g_items.getMaxID();

			std::string searchLower = toLower(searchName);

			for (int id = 1; id <= maxId && count < limit; ++id) {
				if (!g_items.typeExists(id)) {
					continue;
				}

				ItemType& it = g_items.getItemType(id);
				if (it.name.empty()) {
					continue;
				}

				// Check if name contains search string
				if (containsIgnoreCase(it.name, searchName)) {
					sol::table item = lua.create_table();
					item["id"] = it.id;
					item["name"] = it.name;
					results[++count] = item;
				}
			}

			return results;
		};

		// Find first item matching name exactly (case-insensitive)
		// Returns item ID or nil
		items["findIdByName"] = [](const std::string& searchName) -> sol::optional<int> {
			std::string searchLower = toLower(searchName);
			int maxId = g_items.getMaxID();

			// First pass: exact match
			for (int id = 1; id <= maxId; ++id) {
				if (!g_items.typeExists(id)) {
					continue;
				}

				ItemType& it = g_items.getItemType(id);
				if (it.name.empty()) {
					continue;
				}

				if (toLower(it.name) == searchLower) {
					return id;
				}
			}

			// Second pass: contains match (return first)
			for (int id = 1; id <= maxId; ++id) {
				if (!g_items.typeExists(id)) {
					continue;
				}

				ItemType& it = g_items.getItemType(id);
				if (it.name.empty()) {
					continue;
				}

				if (containsIgnoreCase(it.name, searchName)) {
					return id;
				}
			}

			return sol::nullopt;
		};

		// Get item name by ID
		items["getName"] = [](int id) -> std::string {
			if (g_items.typeExists(id)) {
				return g_items.getItemType(id).name;
			}
			return "";
		};
	}

} // namespace LuaAPI
