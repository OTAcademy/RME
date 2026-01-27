//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/ground/auto_border.h"
#include "brushes/brush_enums.h"
#include "brushes/ground/ground_brush.h"
#include "game/items.h"
#include "ext/pugixml.hpp"
#include <wx/string.h>

AutoBorder::AutoBorder(int _id) :
	id(_id), group(0), ground(false) {
	tiles.fill(0);
}

int AutoBorder::edgeNameToID(std::string_view edgename) {
	if (edgename == "n") {
		return NORTH_HORIZONTAL;
	} else if (edgename == "w") {
		return WEST_HORIZONTAL;
	} else if (edgename == "s") {
		return SOUTH_HORIZONTAL;
	} else if (edgename == "e") {
		return EAST_HORIZONTAL;
	} else if (edgename == "cnw") {
		return NORTHWEST_CORNER;
	} else if (edgename == "cne") {
		return NORTHEAST_CORNER;
	} else if (edgename == "csw") {
		return SOUTHWEST_CORNER;
	} else if (edgename == "cse") {
		return SOUTHEAST_CORNER;
	} else if (edgename == "dnw") {
		return NORTHWEST_DIAGONAL;
	} else if (edgename == "dne") {
		return NORTHEAST_DIAGONAL;
	} else if (edgename == "dsw") {
		return SOUTHWEST_DIAGONAL;
	} else if (edgename == "dse") {
		return SOUTHEAST_DIAGONAL;
	}
	return BORDER_NONE;
}

bool AutoBorder::load(pugi::xml_node node, wxArrayString& warnings, GroundBrush* owner, uint16_t ground_equivalent) {
	ASSERT(ground ? ground_equivalent != 0 : true);

	pugi::xml_attribute attribute;

	bool optionalBorder = false;
	if ((attribute = node.attribute("type"))) {
		if (std::string_view(attribute.as_string()) == "optional") {
			optionalBorder = true;
		}
	}

	if ((attribute = node.attribute("group"))) {
		group = attribute.as_ushort();
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (!(attribute = childNode.attribute("item"))) {
			continue;
		}

		uint16_t itemid = attribute.as_ushort();
		if (!(attribute = childNode.attribute("edge"))) {
			continue;
		}

		const std::string_view orientation = attribute.as_string();

		ItemType& it = g_items[itemid];
		if (it.id == 0) {
			warnings.push_back("Invalid item ID " + std::to_string(itemid) + " for border " + std::to_string(id));
			continue;
		}

		if (ground) { // We are a ground border
			it.group = ITEM_GROUP_NONE;
			it.ground_equivalent = ground_equivalent;
			it.brush = owner;

			ItemType& it2 = g_items[ground_equivalent];
			it2.has_equivalent = it2.id != 0;
		}

		it.alwaysOnBottom = true; // Never-ever place other items under this, will confuse the user something awful.
		it.isBorder = true;
		it.isOptionalBorder = it.isOptionalBorder ? true : optionalBorder;
		if (group && !it.border_group) {
			it.border_group = group;
		}

		int32_t edge_id = edgeNameToID(orientation);
		if (edge_id != BORDER_NONE) {
			tiles[edge_id] = itemid;
			if (it.border_alignment == BORDER_NONE) {
				it.border_alignment = ::BorderType(edge_id);
			}
		}
	}
	return true;
}
