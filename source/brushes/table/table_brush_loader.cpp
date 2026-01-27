//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/table/table_brush_loader.h"
#include "brushes/table/table_brush.h"
#include "brushes/table/table_brush_items.h"
#include "game/items.h"
#include <utility>
#include <string_view>

bool TableBrushLoader::load(pugi::xml_node node, TableBrush& brush, TableBrushItems& items, wxArrayString& warnings) {
	uint16_t look_id = 0;
	if (const pugi::xml_attribute attribute = node.attribute("server_lookid")) {
		look_id = g_items[attribute.as_ushort()].clientID;
	}

	if (look_id == 0) {
		look_id = node.attribute("lookid").as_ushort();
	}

	// We need to set items.

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (as_lower_str(childNode.name()) != "table") {
			continue;
		}

		const std::string& alignString = childNode.attribute("align").as_string();
		if (alignString.empty()) {
			warnings.push_back("Could not read type tag of table node\n");
			continue;
		}

		uint32_t alignment = getAlignment(alignString, warnings);
		if (alignment == 0xFFFFFFFF) { // Sentinel for error
			continue;
		}

		for (pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
			if (as_lower_str(subChildNode.name()) != "item") {
				continue;
			}

			uint16_t id = subChildNode.attribute("id").as_ushort();
			if (id == 0) {
				warnings.push_back("Could not read id tag of item node\n");
				break;
			}

			ItemType& it = g_items[id];
			if (it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				return false;
			} else if (it.brush && it.brush != &brush) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				return false;
			}

			it.isTable = true;
			it.brush = &brush;

			items.addItem(alignment, id, subChildNode.attribute("chance").as_int());
		}
	}

	brush.look_id = look_id;

	return true;
}

uint32_t TableBrushLoader::getAlignment(const std::string& alignString, wxArrayString& warnings) {
	static constexpr std::pair<std::string_view, uint32_t> alignments[] = {
		{ "vertical", TABLE_VERTICAL },
		{ "horizontal", TABLE_HORIZONTAL },
		{ "south", TABLE_SOUTH_END },
		{ "east", TABLE_EAST_END },
		{ "north", TABLE_NORTH_END },
		{ "west", TABLE_WEST_END },
		{ "alone", TABLE_ALONE }
	};

	for (const auto& [name, id] : alignments) {
		if (name == alignString) {
			return id;
		}
	}

	warnings.push_back("Unknown table alignment '" + wxstr(alignString) + "'\n");
	return 0xFFFFFFFF;
}
