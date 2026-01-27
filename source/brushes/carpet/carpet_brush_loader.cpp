//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/carpet/carpet_brush_loader.h"
#include "brushes/carpet/carpet_brush.h"
#include "brushes/ground/auto_border.h"
#include "game/items.h"
#include <string_view>
#include <charconv>
#include <algorithm>
#include <cctype>

// Helper for C++20 case-insensitive comparison (zero allocation)
static const auto iequal = [](char a, char b) {
	return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
};

bool CarpetBrushLoader::load(CarpetBrush& brush, pugi::xml_node node, wxArrayString& warnings) {
	pugi::xml_attribute attribute;
	if ((attribute = node.attribute("lookid"))) {
		brush.look_id = attribute.as_ushort();
	}

	if ((attribute = node.attribute("server_lookid"))) {
		uint16_t clientID = g_items[attribute.as_ushort()].clientID;
		brush.look_id = clientID;
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		std::string_view nodeName = childNode.name();

		// Optimize with C++20: Zero allocation comparison
		if (!std::ranges::equal(nodeName, std::string_view("carpet"), iequal)) {
			continue;
		}

		uint32_t alignment;
		if ((attribute = childNode.attribute("align"))) {
			std::string alignString = attribute.as_string();
			alignment = AutoBorder::edgeNameToID(alignString);
			if (alignment == BORDER_NONE) {
				if (alignString == "center") {
					alignment = CARPET_CENTER;
				} else {
					warnings.push_back("Invalid alignment of carpet node\n");
					continue;
				}
			}
		} else {
			warnings.push_back("Could not read alignment tag of carpet node\n");
			continue;
		}

		bool use_local_id = true;
		for (pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
			std::string_view subNodeName = subChildNode.name();
			if (!std::ranges::equal(subNodeName, std::string_view("item"), iequal)) {
				continue;
			}

			use_local_id = false;
			if (!(attribute = subChildNode.attribute("id"))) {
				warnings.push_back("Could not read id tag of item node\n");
				continue;
			}

			int32_t id = attribute.as_int();
			if (!(attribute = subChildNode.attribute("chance"))) {
				warnings.push_back("Could not read chance tag of item node\n");
				continue;
			}

			int32_t chance = attribute.as_int();

			ItemType& it = g_items[id];
			if (it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				continue;
			} else if (it.brush && it.brush != &brush) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				continue;
			}

			it.isCarpet = true;
			it.brush = &brush;

			brush.m_items.addItem(static_cast<BorderType>(alignment), id, chance);
		}

		if (use_local_id) {
			if (!(attribute = childNode.attribute("id"))) {
				warnings.push_back("Could not read id tag of carpet node\n");
				continue;
			}

			uint16_t id = attribute.as_ushort();

			ItemType& it = g_items[id];
			if (it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				return false;
			} else if (it.brush && it.brush != &brush) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				return false;
			}

			it.isCarpet = true;
			it.brush = &brush;

			brush.m_items.addItem(static_cast<BorderType>(alignment), id, 1);
		}
	}
	return true;
}
