//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/doodad/doodad_brush_loader.h"
#include "brushes/doodad/doodad_brush_items.h"
#include "brushes/doodad/doodad_brush.h" // For DoodadBrush definition if needed for pointer check?
// actually we just pass DoodadBrush*
#include "game/item.h" // For g_items
#include "game/items.h"
#include "map/tile.h"

#include "ext/pugixml.hpp"
#include <boost/lexical_cast.hpp>

// Helper
#include <ranges>
#include <algorithm>
#include <cctype>
#include <string_view>

// Helper for C++20 case-insensitive comparison (zero allocation)
static const auto iequal = [](char a, char b) {
	return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
};

bool DoodadBrushLoader::load(pugi::xml_node node, DoodadBrushItems& items, DoodadBrushSettings& settings, wxArrayString& warnings, DoodadBrush* brushPtr) {
	pugi::xml_attribute attribute;

	if ((attribute = node.attribute("lookid"))) {
		settings.look_id = attribute.as_ushort();
	}

	if ((attribute = node.attribute("server_lookid"))) {
		settings.look_id = g_items[attribute.as_ushort()].clientID;
	}

	if ((attribute = node.attribute("on_blocking"))) {
		settings.on_blocking = attribute.as_bool();
	}

	if ((attribute = node.attribute("on_duplicate"))) {
		settings.on_duplicate = attribute.as_bool();
	}

	if ((attribute = node.attribute("redo_borders")) || (attribute = node.attribute("reborder"))) {
		settings.do_new_borders = attribute.as_bool();
	}

	if ((attribute = node.attribute("one_size"))) {
		settings.one_size = attribute.as_bool();
	}

	if ((attribute = node.attribute("draggable"))) {
		settings.draggable = attribute.as_bool();
	}

	if (node.attribute("remove_optional_border").as_bool()) {
		if (!settings.do_new_borders) {
			warnings.push_back("remove_optional_border will not work without redo_borders\n");
		}
		settings.clear_statflags |= TILESTATE_OP_BORDER;
	}

	const std::string& thicknessString = node.attribute("thickness").as_string();
	if (!thicknessString.empty()) {
		size_t slash = thicknessString.find('/');
		if (slash != std::string::npos) {
			try {
				settings.thickness = boost::lexical_cast<int32_t>(thicknessString.substr(0, slash));
				settings.thickness_ceiling = std::max<int32_t>(settings.thickness, boost::lexical_cast<int32_t>(thicknessString.substr(slash + 1)));
			} catch (const boost::bad_lexical_cast&) {
				warnings.push_back("Invalid thickness format: " + wxstr(thicknessString));
				settings.thickness = 0;
				settings.thickness_ceiling = 0;
			}
		}
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (!std::ranges::equal(std::string_view(childNode.name()), std::string_view("alternate"), iequal)) {
			continue;
		}
		if (!loadAlternative(childNode, items, warnings, nullptr, brushPtr)) {
			return false;
		}
	}

	const auto& alternatives = items.getAlternatives();
	loadAlternative(node, items, warnings, alternatives.empty() ? nullptr : alternatives.back().get(), brushPtr);
	return true;
}

bool DoodadBrushLoader::loadAlternative(pugi::xml_node node, DoodadBrushItems& items, wxArrayString& warnings, DoodadBrushItems::AlternativeBlock* which, DoodadBrush* brushPtr) {
	DoodadBrushItems::AlternativeBlock* alternativeBlock;
	if (which) {
		alternativeBlock = which;
	} else {
		alternativeBlock = items.createAlternative();
	}

	pugi::xml_attribute attribute;
	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		std::string_view childName = childNode.name();
		if (std::ranges::equal(childName, std::string_view("item"), iequal)) {
			if (!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			Item* item = Item::Create(childNode);
			if (!item) {
				warnings.push_back("Can't create item from doodad item node.");
				continue;
			}

			ItemType& it = g_items[item->getID()];
			if (it.id != 0 && brushPtr) {
				it.doodad_brush = brushPtr;
			}

			items.addSingleToBlock(alternativeBlock, std::unique_ptr<Item>(item), attribute.as_int());

		} else if (std::ranges::equal(childName, std::string_view("composite"), iequal)) {
			if (!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			int compositeChance = attribute.as_int();
			CompositeTileList compositeList;

			for (pugi::xml_node compositeNode = childNode.first_child(); compositeNode; compositeNode = compositeNode.next_sibling()) {
				if (!std::ranges::equal(std::string_view(compositeNode.name()), std::string_view("tile"), iequal)) {
					continue;
				}

				if (!(attribute = compositeNode.attribute("x"))) {
					warnings.push_back("Couldn't read positionX values of composite tile node.");
					continue;
				}

				int32_t x = attribute.as_int();
				if (!(attribute = compositeNode.attribute("y"))) {
					warnings.push_back("Couldn't read positionY values of composite tile node.");
					continue;
				}

				int32_t y = attribute.as_int();
				int32_t z = compositeNode.attribute("z").as_int();
				if (x < -0x7FFF || x > 0x7FFF) {
					warnings.push_back("Invalid range of x value on composite tile node.");
					continue;
				} else if (y < -0x7FFF || y > 0x7FFF) {
					warnings.push_back("Invalid range of y value on composite tile node.");
					continue;
				} else if (z < -0x7 || z > 0x7) {
					warnings.push_back("Invalid range of z value on composite tile node.");
					continue;
				}

				DoodadItemVector tiles_items;
				for (pugi::xml_node itemNode = compositeNode.first_child(); itemNode; itemNode = itemNode.next_sibling()) {
					if (!std::ranges::equal(std::string_view(itemNode.name()), std::string_view("item"), iequal)) {
						continue;
					}

					Item* item = Item::Create(itemNode);
					if (item) {
						tiles_items.push_back(std::unique_ptr<Item>(item));

						ItemType& it = g_items[item->getID()];
						if (it.id != 0 && brushPtr) {
							it.doodad_brush = brushPtr;
						}
					} else {
						warnings.push_back("Can't create item from doodad composite tile node.");
					}
				}

				if (!tiles_items.empty()) {
					compositeList.push_back(std::make_pair(Position(x, y, z), std::move(tiles_items)));
				}
			}

			if (!compositeList.empty()) {
				items.addCompositeToBlock(alternativeBlock, compositeList, compositeChance);
			}
		}
	}

	return true;
}
